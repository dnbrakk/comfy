/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "comfy.h"
#include <future>
#include <queue>
#include <list>
#include <mutex> 
#include <shared_mutex>
#include <functional>
#include <thread>

template <typename T>
struct threadsafe_queue;

typedef threadsafe_queue<std::function<void()>> function_queue;


static const std::string DEFAULT_JOB_POOL_ID = "DEFAULT";


struct checkout_token
{
    checkout_token(
        std::function<void()> _check_in
    )
    : b_neutralized(false)
    , check_in(_check_in)
    {}

    ~checkout_token()
    {
        if (!b_neutralized)
        {
            check_in();
        }
    }


    bool b_neutralized;
    std::function<void()> check_in;


    void neutralize()
    {
        b_neutralized = true;
    }

};


// isn't actually fully threadsafe with regards
// to the get() returning a non-const value
template <typename K, typename V>
struct threadsafe_map
{
    threadsafe_map()
    : map(std::map<K, V>())
    {}


    std::map<K, V> map;
    std::shared_mutex mtx;
    std::vector<K> checked_out;


    static void check_in_callback(threadsafe_map<K, V>* ts_m, K& k)
    {
        if (!ts_m) return;
        ts_m->check_in(k);
    }

    // returns true if the item was removed
    // from the map, which occurs if there
    // exist no other checkout_tokens for the key
    bool check_in(K& k)
    {
        {
            std::unique_lock<std::shared_mutex> lck(mtx);

            bool b_remove = false;
            auto it = std::find(checked_out.begin(), checked_out.end(), k);
            if (it != checked_out.end())
            {
                checked_out.erase(it);
                it = std::find(checked_out.begin(), checked_out.end(), k);
                if (it == checked_out.end())
                {
                    b_remove = true;
                }
            }

            if (b_remove)
            {
                map.erase(k);
                return true;
            }
        }

        return false;
    }

    bool is_checked_out(K& k)
    {
        std::shared_lock<std::shared_mutex> lck(mtx);

        auto it = std::find(checked_out.begin(), checked_out.end(), k);
        if (it != checked_out.end())
        {
            return true;
        }

        return false;
    }

    // as long as at least one checkout token
    // for a map value with key k exists,
    // the value associated with that key
    // cannot be removed from the map.
    // when the checkout_token destructs,
    // it checks the value with key k back
    // in, and if no other checkout_tokens
    // for that value exist, the value is
    // removed from the map
    std::shared_ptr<checkout_token> checkout(K& k)
    {
        std::unique_lock<std::shared_mutex> lck(mtx);

        checked_out.push_back(k);
        return std::make_shared<checkout_token>(
            std::bind(check_in_callback, this, k));
    }

    // does not return a reference to the inserted value,
    // as modifying the inserted value after it has been
    // added can cause race conditions or segfaults
    void insert(K& k, V& v)
    {
        std::unique_lock<std::shared_mutex> lck(mtx);

        map[k] = v;
    }

    // e.g. to prevent the destructor of v from
    //      being called after a copy assignment
    void emplace_move(K& k, V& v)
    {
        std::unique_lock<std::shared_mutex> lck(mtx);

        map.emplace(k, std::move(v));
    }

    size_t remove(K& k)
    {
        if (!is_checked_out(k))
        {
            std::unique_lock<std::shared_mutex> lck(mtx);
            return map.erase(k);
        }

        return 0;
    }

    V* get(K& k)
    {
        std::shared_lock<std::shared_mutex> lck(mtx);

        try
        {
            V& v = map.at(k);
            return &v;
        }
        catch (const std::out_of_range& oor)
        {
            return nullptr;
        }

        return nullptr;
    }
};


// thread safe queue
template <typename T>
struct threadsafe_queue
{
    threadsafe_queue()
    : queue(std::queue<T>())
    , id(DEFAULT_JOB_POOL_ID)
    , c_token(nullptr)
    {}

    threadsafe_queue(
        std::string _id
    )
    : queue(std::queue<T>())
    , id(_id)
    , c_token(nullptr)
    {}

    threadsafe_queue(
        const threadsafe_queue& other
    )
    {
        *this = other;
    }


    std::queue<T> queue;
    std::mutex q_m;
    std::condition_variable c_v;
    std::string id;
    // can be used to self-remove from list or map
    std::shared_ptr<checkout_token> c_token;


    std::string get_id() const
    {
        return id;
    }


    bool has_id(const std::string& _id)
    {
        if (id.compare(_id) == 0)
        {
            return true;
        }

        return false;
    }


    void push(T& t)
    {
        {
            std::lock_guard<std::mutex> lock(q_m);

            queue.push(std::move(t));
        }

        c_v.notify_one();
    }


    bool try_pop(T& t, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(q_m);

        if (!c_v.wait_for(lock, timeout, [this] { return !queue.empty(); }))
        {
            return false;
        }

        t = std::move(queue.front());
        queue.pop();

        // potentially trigger checkout_token checkin()
        if (queue.empty())
        {
            c_token = nullptr;
        }

        return true;    
    }


    void front(T& t, std::chrono::milliseconds timeout)
    {
        {
            std::lock_guard<std::mutex> lock(q_m);

            t = queue.front();
        }

        c_v.notify_one();
    }


    bool empty()
    {
        bool b_empty = false;

        {
            std::lock_guard<std::mutex> lock(q_m);

            b_empty = queue.empty();
        }

        c_v.notify_one();

        return b_empty;
    }


    void set_checkout_token(std::shared_ptr<checkout_token> t)
    {
        c_token = t;
    }


    inline void operator=(const threadsafe_queue& other)
    {
        id = other.id;
        queue = other.queue;
        c_token = other.c_token;
    }
};


template<typename T, typename K>
struct threadsafe_list
{
    threadsafe_list()
    : list(std::list<std::shared_ptr<T>>())
    , checked_out(std::vector<K>())
    {}


    std::list<std::shared_ptr<T>> list;
    std::mutex list_mtx;
    std::condition_variable c_v;
    // checkout items
    std::vector<K> checked_out;


    static void check_in_callback(threadsafe_list<T, K>* ts_l, const K& k)
    {
        if (!ts_l) return;
        ts_l->check_in(k);
    }


    // returns true if the item was removed
    // from the map, which occurs if there
    // exist no other checkout_tokens for the key
    bool check_in(const K& k)
    {
        {
            std::lock_guard<std::mutex> lock(list_mtx);

            auto it = std::find(checked_out.begin(), checked_out.end(), k);
            if (it != checked_out.end())
            {
                checked_out.erase(it);
            }

            // MUTEX destructs
        }

        c_v.notify_one();

        // delete from list
        if (!is_checked_out(k))
        {
            remove(k);
            return true;
        }

        return false;
    }


    bool is_checked_out(const K& k)
    {
        bool b_out = false;

        {
            std::lock_guard<std::mutex> lock(list_mtx);

            auto it = std::find(checked_out.begin(), checked_out.end(), k);
            if (it != checked_out.end())
            {
                b_out = true;
            }
        }

        c_v.notify_one();

        return b_out;
    }


    // as long as at least one checkout token
    // for a map value with key k exists,
    // the value associated with that key
    // cannot be removed from the map.
    // when the checkout_token destructs,
    // it checks the value with key k back
    // in, and if no other checkout_tokens
    // for that value exist, the value is
    // removed from the map
    std::shared_ptr<checkout_token> checkout(const K& k)
    {
        {
            std::lock_guard<std::mutex> lock(list_mtx);
            checked_out.push_back(k);
        }

        c_v.notify_one();

        return std::make_shared<checkout_token>(
            std::bind(check_in_callback, this, k));
    }


    // adds a checkout token to the inserted item
    // so that it can self-remove from the map
    void push_front_and_self_checkout(std::shared_ptr<T>& t)
    {
        if (!t) return;
        // do not add duplicates
        if (get(t->get_id())) return;

        {
            std::lock_guard<std::mutex> lock(list_mtx);

            list.push_front(t);
            std::shared_ptr<T>& ta = list.front();
            std::string k = ta->get_id();
            ta->set_checkout_token(std::make_shared<checkout_token>(
                std::bind(check_in_callback, this, k)));
            checked_out.push_back(k);
        }

        c_v.notify_one();
    }


    // adds a checkout token to the inserted item
    // so that it can self-remove from the map
    void push_back_and_self_checkout(std::shared_ptr<T>& t)
    {
        if (!t) return;
        // do not add duplicates
        if (get(t->get_id())) return;

        {
            std::lock_guard<std::mutex> lock(list_mtx);

            list.push_back(t);
            std::shared_ptr<T>& ta = list.back();
            std::string k = ta->get_id();
            ta->set_checkout_token(std::make_shared<checkout_token>(
                std::bind(check_in_callback, this, k)));
            checked_out.push_back(k);
        }

        c_v.notify_one();
    }


    void push_back(std::shared_ptr<T>& t)
    {
        if (!t) return;

        {
            std::lock_guard<std::mutex> lock(list_mtx);

            list.push_back(t);
        }

        c_v.notify_one();
    }


    void push_front(std::shared_ptr<T>& t)
    {
        if (!t) return;

        {
            std::lock_guard<std::mutex> lock(list_mtx);

            list.push_front(t);
        }

        c_v.notify_one();
    }


    bool try_pop_front(std::shared_ptr<T>& t, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(list_mtx);

        if (!c_v.wait_for(lock, timeout, [this] { return !list.empty(); }))
        {
            return false;
        }

        t = list.front();
        list.pop_front();

        return true;    
    }


    std::shared_ptr<T> front()
    {
        std::shared_ptr<T> tf = nullptr;

        // first get front item id
        {
            std::lock_guard<std::mutex> lock(list_mtx);

            if (!list.empty())
            {
                tf = list.front();
            }
        }

        c_v.notify_one();

        return tf;
    }


    void move_to_front(const K& k)
    {
        std::shared_ptr<T> t = get(k);
        remove(k);
        push_front_and_self_checkout(t);
    }


    void move_to_back(const K& k)
    {
        std::shared_ptr<T> t = get(k);
        remove(k);
        push_back_and_self_checkout(t);
    }


    void remove(const K& k)
    {
        {
            std::lock_guard<std::mutex> lock(list_mtx);

            for (auto it = list.begin(); it != list.end(); ++it)
            {
                if (*it && (*it)->has_id(k))
                {
                    // neutralize checkout_token so it does not
                    // try te remove the time from the list
                    // upon destruction
                    if ((*it)->c_token)
                    {
                        (*it)->c_token->neutralize();
                    }

                    it = list.erase(it);
                }
            }

            // remove all instances of item id in checked_out
            auto it = std::find(checked_out.begin(), checked_out.end(), k);
            while (it != checked_out.end())
            {
                checked_out.erase(it);
                it = std::find(checked_out.begin(), checked_out.end(), k);
            }

            // Mutex destructs
        }

        c_v.notify_one();
    }


    std::shared_ptr<T> get(const K& k)
    {
        std::shared_ptr<T> t = nullptr;

        {
            std::lock_guard<std::mutex> lock(list_mtx);

            for (auto it = list.begin(); it != list.end(); ++it)
            {
                if (*it && (*it)->has_id(k))
                {
                    t = *it;
                    break;
                }
            }
        }

        c_v.notify_one();

        return t;
    }


    int size()
    {
        int size = 0;

        {
            std::lock_guard<std::mutex> lock(list_mtx);

            size = list.size();
        }

        c_v.notify_one();

        return size;
    }


    bool empty()
    {
        bool b_empty = false;

        {
            std::lock_guard<std::mutex> lock(list_mtx);

            b_empty = list.empty();
        }

        c_v.notify_one();

        return b_empty;
    }


    void clear()
    {
        {
            std::lock_guard<std::mutex> lock(list_mtx);

            for (auto it = list.begin(); it != list.end(); ++it)
            {
                if (*it && (*it)->c_token)
                {
                    (*it)->c_token->neutralize();
                }
            }

            checked_out.clear();
            list.clear();
        }

        c_v.notify_one();
    }

};


class ThreadMan
{
public:
// leave the constructor empty so that nothing is executed when the static instance is fetched
    ThreadMan() {};

    // delete these functions for use as singleton
    ThreadMan(ThreadMan const&)         = delete;
    void operator=(ThreadMan const&)    = delete;

    static ThreadMan& get_instance()
    {
        static ThreadMan thread_man;
        return thread_man;
    }

    void init(int _max_threads = -1)
    {
        if (_max_threads > 0)
        {
            MAX_THREADS = _max_threads;
        }
        else
        {
            MAX_THREADS = std::thread::hardware_concurrency() - 1;
            if (MAX_THREADS < 1)
            {
                MAX_THREADS = 1;
            }
        }

        thread_semaphore = MAX_THREADS;
    }


    int MAX_THREADS;

    // mutexes and semaphores
    std::shared_mutex s_mtx;
    int thread_semaphore;

    threadsafe_list<function_queue, std::string> job_pool_list;

    bool take_thread_semaphore();
    void replace_thread_semaphore();
    int get_thread_semaphore();

    void kill_jobs(const std::string& job_pool_id);
    void move_jobs_to_front(const std::string& job_pool_id);
    void move_jobs_to_back(const std::string& job_pool_id);
    void enqueue_job(std::function<void()> job, const std::string& job_pool_id = DEFAULT_JOB_POOL_ID, bool b_push_to_front = false);
    static void start_thread(); // consumes jobs
    static void run_jobs();
    static void end_thread();

    void shutdown();

};

