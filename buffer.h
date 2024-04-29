/*
 *
 * Authors: Meghan Grayson and Vaishnavi Karaguppi
 * Date: April 29, 2024
 * 
 * This is the class definition for a circular buffer with producer-consumer conditional variables.
 * 
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <condition_variable>
#include <mutex>
#include <cassert>
#include <cmath>
#include <cstdlib>

namespace producerConsumer {
    // Provides a circular buffer than can be used in a producer-consumer manner
    class buffer {
        private:
            int head; // The index of the buffer "head"
            int tail; // The index of the buffer "tail"
            int count; // The number of items currently in the buffer
            int capacity; // The size of the buffer
            std::string *buff; // The actual container of items
            /* Condition variables and mutexes */
            std::mutex pcLock; // The lock to be used to add or remove from the buffer
            std::condition_variable notFull; // The condition to indicate the buffer can be added to
            std::condition_variable notEmpty; // The condition to indicate the buffer can be removed from
        public:
            // Buffer constructor
            buffer(int bufSize);

            // Buffer destructor
            ~buffer();

            // Function to add an item to the buffer
            void add(std::string item);

            // Function to remove an item from the buffer
            std::string remove(void);
    };
}

#endif