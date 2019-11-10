/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "threadman.h"


  ////////////////
 // semaphores //
////////////////

bool ThreadMan::take_thread_semaphore()
{
    std::unique_lock<std::shared_mutex> lck(s_mtx);

    if (thread_semaphore > 0)
    {
        thread_semaphore--;
        return true;
    }

    return false;
}


void ThreadMan::replace_thread_semaphore()
{
    std::unique_lock<std::shared_mutex> lck(s_mtx);

    if (thread_semaphore < MAX_THREADS)
    {
        thread_semaphore++;
    }
}


int ThreadMan::get_thread_semaphore()
{
    std::unique_lock<std::shared_mutex> lck(s_mtx);
    return thread_semaphore;
}


  //////////
 // jobs //
//////////

void ThreadMan::enqueue_job(std::function<void()> job, const std::string& job_pool_id, bool b_push_to_front)
{
    std::shared_ptr<function_queue> queue = job_pool_list.get(job_pool_id);
    if (queue)
    {
        queue->push(job);
        if (b_push_to_front)
        {
            // remove from list then push to front of list
            job_pool_list.remove(queue->get_id());
            job_pool_list.push_front_and_self_checkout(queue);
        }
        else
        {
            // ensure that queue has not been removed from job pool
            // in the time it took to get it and push the job to the queue
            // (duplicates are not allowed, so if the queue still exists
            // in the job pool then push_back_and_self_checkout()
            // returns immediately)
            job_pool_list.push_back_and_self_checkout(queue);
        }
    }
    else
    {
        std::shared_ptr<function_queue> new_q =
            std::make_shared<function_queue>(job_pool_id);
        new_q->push(job);
        if (b_push_to_front)
        {
            job_pool_list.push_front_and_self_checkout(new_q);
        }
        else
        {
            job_pool_list.push_back_and_self_checkout(new_q);
        }
    }

    // free thread exists, start another to run jobs
    if (take_thread_semaphore())
    {
        std::thread t(start_thread);
        t.detach();
    }
}


void ThreadMan::kill_jobs(const std::string& job_pool_id)
{
    job_pool_list.remove(job_pool_id);
}


void ThreadMan::move_jobs_to_front(const std::string& job_pool_id)
{
    job_pool_list.move_to_front(job_pool_id); 
}


void ThreadMan::move_jobs_to_back(const std::string& job_pool_id)
{
    job_pool_list.move_to_back(job_pool_id); 
}


void ThreadMan::start_thread()
{
    run_jobs();
    end_thread();
    // << thread terminates
}


void ThreadMan::run_jobs()
{
    using namespace std::chrono;

    // run jobs on this thread until there are no jobs left
    std::shared_ptr<function_queue> queue = THREAD_MAN.job_pool_list.front();
    while (queue)
    {
        std::function<void()> job;
        if (queue->try_pop(job, milliseconds(10)))
        {
            queue = nullptr;
            job();
        }

        queue = THREAD_MAN.job_pool_list.front();
    }
}


void ThreadMan::end_thread()
{
    THREAD_MAN.replace_thread_semaphore();
}


void ThreadMan::shutdown()
{
    job_pool_list.clear();

    // wait for all threads to finish
    while (get_thread_semaphore() < MAX_THREADS)
    {}
}

