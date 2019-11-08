/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "googlecaptcha.h"
#include "netops.h"

GoogleCaptchaSolution::GoogleCaptchaSolution() :
    boxes({false})
{

}

// Returns false if the column or row is outside the grid
bool GoogleCaptchaSolution::select_box(unsigned int column, unsigned int row, BoxAction action)
{
    if(column > 2 || row > 2)
        return false;

    unsigned int box_index = column + 3 * row;
    switch(action)
    {
        case BoxAction::TICK:
            boxes[box_index] = true;
            break;
        case BoxAction::UNTICK:
            boxes[box_index] = false;
            break;
        case BoxAction::TOGGLE:
            boxes[box_index] = !boxes[box_index];
            break;
    }
}

std::string GoogleCaptchaSolution::build_http_post_data() const
{
    std::string result;
    for(size_t i = 0; i < boxes.size(); ++i)
    {
        if(boxes[i])
        {
            if(!result.empty())
                result += "&";
            result += "response=" + std::to_string(i);
        }
    }
    return result;
}

GoogleCaptcha::GoogleCaptcha(const std::string& _api_key, const std::string& _referer) : 
    api_key(_api_key),
    referer(_referer),
    notifying_subscriptions(false),
    notify_new_challenge(false),
    notify_solved(false)
{

}

GoogleCaptcha& GoogleCaptcha::get_4chan_instance()
{
    // (dec05eba): I'm not sure if this api key is always valid, but it seems to have remained the same for a long time.
    // If showing captcha in the future fails, then this api key might have been revoked by 4chan
    static GoogleCaptcha fourchan_instance("6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc", "https://boards.4chan.org/");
    return fourchan_instance;
}

bool GoogleCaptcha::subscribe(Delegate* delegate)
{
    assert(!notifying_subscriptions);
    if(notifying_subscriptions)
        return false;

    for(Delegate* subscription : subscriptions)
    {
        if(subscription == delegate)
            return false;
    }
    subscriptions.push_back(delegate);
    return true;
}

bool GoogleCaptcha::unsubscribe(Delegate* delegate)
{
    assert(!notifying_subscriptions);
    if(notifying_subscriptions)
        return false;

    for(auto it = subscriptions.begin(), end = subscriptions.end(); it != end; ++it)
    {
        if(*it == delegate)
        {
            subscriptions.erase(it);
            return true;
        }
    }
    return false;
}

bool GoogleCaptcha::submit_captcha_solution(GoogleCaptchaSolution solution)
{
    std::lock_guard<std::mutex> lock(challenge_update_mutex);
    if(state != State::SOLVING_CAPTCHA)
        return false;
    state = State::SUBMITTING_SOLUTION;
    NetOps::http_post__google_captcha_solution(api_key, challenge.id, std::move(solution),
        std::bind(&GoogleCaptcha::on_receive_solution_response, this, std::placeholders::_1));
}

// TODO: The google captcha key is only valid for 2 minutes and right now
// and if the user tries to post after that and it will show a new captcha image.
// A more user friendly solution would be to invalidate the captcha key
// and request a new google captcha image automatically.
void GoogleCaptcha::tick()
{
    if(subscriptions.empty())
        return;

    switch(state)
    {
        case State::NONE:
        case State::INVALID:
        {
            state = State::RECEIVING_NEW_CAPTCHA;
            NetOps::http_get__google_captcha(api_key, referer,
                std::bind(&GoogleCaptcha::on_receive_new_challenge, this, std::placeholders::_1));
            break;
        }
        case State::SOLVING_CAPTCHA:
        {
            if(notify_new_challenge)
            {
                notify_new_challenge = false;
                GoogleCaptchaChallenge new_challenge = get_captcha_challenge();
                notifying_subscriptions = true;
                for(Delegate* subscription : subscriptions)
                {
                    subscription->on_receive_new_challenge(new_challenge);
                }
                notifying_subscriptions = false;
            }
            break;
        }
        case State::CAPTCHA_SOLVED:
        {
            if(notify_solved)
            {
                notify_solved = false;
                std::string new_solved_captcha_id = get_solved_captcha_id();
                notifying_subscriptions = true;
                for(Delegate* subscription : subscriptions)
                {
                    subscription->on_captcha_solved(new_solved_captcha_id);
                }
                notifying_subscriptions = false;
                break;
            }
            break;
        }
    }
}

void GoogleCaptcha::request_state(Delegate* delegate)
{
    switch(state)
    {
        case State::SUBMITTING_SOLUTION:
        case State::SOLVING_CAPTCHA:
        {
            GoogleCaptchaChallenge new_challenge = get_captcha_challenge();
            delegate->on_receive_new_challenge(new_challenge);
            break;
        }
        case State::CAPTCHA_SOLVED:
        {
            std::string new_solved_captcha_id = get_solved_captcha_id();
            delegate->on_captcha_solved(new_solved_captcha_id);
            break;
        }
    }
}

void GoogleCaptcha::on_receive_new_challenge(std::experimental::optional<GoogleCaptchaChallenge> _challenge)
{
    std::lock_guard<std::mutex> lock(challenge_update_mutex);
    if(_challenge)
    {
        challenge = std::move(*_challenge);
        state = State::SOLVING_CAPTCHA;
        notify_new_challenge = true;
    }
    else
    {
        // TODO: Should there be a delay between invalid state and fetching new captcha challenge?
        // Right now when state is set to INVALID, on the next tick() a new challenge will be
        // requested.
        state = State::INVALID;
    }
}

void GoogleCaptcha::on_receive_solution_response(std::experimental::optional<std::string> _solved_captcha_id)
{
    std::lock_guard<std::mutex> lock(challenge_update_mutex);
    if(_solved_captcha_id)
    {
        solved_captcha_id = std::move(*_solved_captcha_id);
        state = State::CAPTCHA_SOLVED;
        notify_solved = true;
    }
    else
    {
        state = State::INVALID;
    }
}

GoogleCaptchaChallenge GoogleCaptcha::get_captcha_challenge()
{
    std::lock_guard<std::mutex> lock(challenge_update_mutex);
    return challenge;
}

std::string GoogleCaptcha::get_solved_captcha_id()
{
    std::lock_guard<std::mutex> lock(challenge_update_mutex);
    return solved_captcha_id;
}
