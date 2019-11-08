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
#include "imgman.h"
#include <functional>

struct GoogleCaptchaChallenge
{
    std::string id;
    std::string description;
    img_packet image;
};

using RequestGoogleCaptchaCallback = std::function<void(std::experimental::optional<GoogleCaptchaChallenge>)>;
using SubmitGoogleCaptchaSolutionCallback = std::function<void(std::experimental::optional<std::string>)>;

// Google captcha is a 3x3 grid, so the column is a value between [0, 2]
// and the row is a value between [0, 2].
class GoogleCaptchaSolution
{
public:
    enum class BoxAction
    {
        TICK,
        UNTICK,
        TOGGLE
    };
    
    GoogleCaptchaSolution();

    // Returns false if the column or row is outside the grid.
    // The column and row should both be a value between [0, 2].
    bool select_box(unsigned int column, unsigned int row, BoxAction action);
    std::string build_http_post_data() const;
private:
    std::array<bool, 9> boxes;
};

// How to use:
// First make your widget inherit GoogleCaptcha::Delegate and then when creating the widget, after it has been initialized:
// `GoogleCaptcha::get_4chan_instance().subscribe(widget);` to receive captcha updates in the future.
// `GoogleCaptcha::get_4chan_instance().request_state(widget)` to receive the current state of the captcha right now.
// and then when you want to submit a solution for the captcha, call `GoogleCaptcha::get_4chan_instance().submit_captcha_solution(...);`
// when the widget should be destroyed, call `GoogleCaptcha::get_4chan_instance().unsubscribe(widget);`
class GoogleCaptcha
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;
        virtual void on_receive_new_challenge(const GoogleCaptchaChallenge& challenge) = 0;
        virtual void on_captcha_solved(const std::string& captcha_id) = 0;
    };

    static GoogleCaptcha& get_4chan_instance();

    bool subscribe(Delegate* delegate);
    bool unsubscribe(Delegate* delegate);
    // If the solution was correct, then Delegate::on_captcha_solved is called, otherwise Delegate::on_receive_new_challenge is called.
    // Returns false if the captcha is not in the process of being solved.
    bool submit_captcha_solution(GoogleCaptchaSolution solution);
    void tick();
    void request_state(Delegate* delegate);
private:
    ~GoogleCaptcha() = default;
    GoogleCaptcha(const GoogleCaptcha&) = delete;
    GoogleCaptcha operator=(const GoogleCaptcha&) = delete;
private:
    enum class State
    {
        NONE,
        RECEIVING_NEW_CAPTCHA,
        SUBMITTING_SOLUTION,
        SOLVING_CAPTCHA,
        CAPTCHA_SOLVED,
        INVALID
    };

    GoogleCaptcha(const std::string& _api_key, const std::string& _referer);
    void on_receive_new_challenge(std::experimental::optional<GoogleCaptchaChallenge> _challenge);
    void on_receive_solution_response(std::experimental::optional<std::string> _solved_captcha_id);
    GoogleCaptchaChallenge get_captcha_challenge();
    std::string get_solved_captcha_id();

    State state;
    std::string solved_captcha_id;
    GoogleCaptchaChallenge challenge;
    std::vector<Delegate*> subscriptions;
    bool notifying_subscriptions;

    std::string api_key;
    std::string referer;

    bool notify_new_challenge;
    bool notify_solved;
    std::mutex challenge_update_mutex;
};
