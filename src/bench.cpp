/*
 * Simple benchmark application using redis pub sub event system
 * This application requires libevent-dev libhiredis & cxxopts.
 * hiredis is an opensource library that provides C bindings for redis.
 * cxxopts is an opensource header only library that provides simple ways to handle commandline arguments.
 */

#include <chrono>

#include <iostream>
#include <string>
#include <cstring>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <cxxopts.hpp>

using namespace std;

typedef chrono::high_resolution_clock Time;
typedef chrono::duration<double> fsec;
typedef chrono::milliseconds ms;

bool timerStarted = false;
chrono::system_clock::time_point startTime;
chrono::system_clock::time_point endTime;

void startTimer(redisAsyncContext *context, void *rep, void *data)
{
    redisReply *reply = (redisReply*)rep;
    if (reply == NULL) {
        cerr << "No message recieved." << endl;
    }

    if(reply->type == REDIS_REPLY_ARRAY & reply->elements == 3)
    {
        if (strcmp( reply->element[0]->str, "subscribe") != 0)
        {
            //cerr << "Message recieved on channel `" << reply->element[1]->str << "`" << endl;
            startTime = Time::now();
            timerStarted = true;
        }
    }
}

void stopTimer(redisAsyncContext *context, void *rep, void *data)
{
    redisReply *reply = (redisReply*)rep;
    if (reply == NULL){
        cout <<"Response not recevied" << endl;
        return;
    }

    if(reply->type == REDIS_REPLY_ARRAY & reply->elements == 3)
    {
        if(strcmp( reply->element[0]->str,"subscribe") != 0)
        {
            cerr << "Message recieved on channel `" << reply->element[1]->str << "`" << endl;
            if (!timerStarted)
            {
                cerr << "Error: Recieved stopTimer event without a previous startTimer event." << endl;
                return;
            }
            endTime = Time::now();
            timerStarted = false;

            // TODO: Move
            fsec total_time = endTime - startTime;
            cout << "Time (ms): " << total_time.count() * 1e3 << endl;
        }
    }
}

int main (int argc, char** argv)
{
    cxxopts::Options options("redis_bench", "Simple program used to measure time between two redis events.");
    options.add_options()
            ("d, debug", "Enable debugging")
            ("s, start-key", "The key that triggers the event to start the timer.", cxxopts::value<string>())
            ("e, end-key", "The key that triggers the event to stop the timer.", cxxopts::value<string>())
            ("h, help", "Print help")
            ;

    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        cout << options.help({"", "Group"});
        return EXIT_FAILURE;
    }

    std::string startKey, endKey;
    if (result.count("s")) { startKey = result["s"].as<string>(); }
    if (result.count("e")) { endKey = result["e"].as<string>(); }

    if (startKey.empty() || endKey.empty())
    {
        cout << "ERROR: Both start and end keys args must be specified to run " << argv[0] << "." << endl << endl;
        cout << options.help({"", "Group"});
        return EXIT_FAILURE;
    }

    struct event_base* event = event_base_new();
    redisAsyncContext* asyncContext = redisAsyncConnect("127.0.0.1", 6379);
    if (asyncContext->err) {
        redisAsyncFree(asyncContext);
        cout <<"Error: "<< asyncContext->errstr << endl;
        return EXIT_FAILURE;
    }

    redisLibeventAttach(asyncContext, event);

    string command = "subscribe " + startKey;
    redisAsyncCommand(asyncContext, startTimer, (char*)"sub", command.c_str());

    command = "subscribe " + endKey;
    redisAsyncCommand(asyncContext, stopTimer, (char*)"sub", command.c_str());

    event_base_dispatch(event);
    redisAsyncFree(asyncContext);
    return EXIT_SUCCESS;
}

