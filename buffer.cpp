/*
 *
 * Authors: Meghan Grayson and Vaishnavi Karaguppi
 * Date: April 29, 2024
 * 
 * This is the class implementation for a circular buffer with producer-consumer conditional variables.
 * 
 */

#include "buffer.h"
#include <iostream>

namespace producerConsumer {
    /*
     * The constructor for the buffer.
     * Initializes the values of member variables as needed
     *    @param bufSize -- an integer representing the size of the buffer
     *    @returns -- none
     */
    buffer::buffer(int bufSize) {
        head = 0; // Initialize all counts
        tail = 0;
        count = 0;
        capacity = bufSize; // Set the buffer capacity
        buff = new std::string[capacity];
        /* Fill the buffer with empty strings */
        for(int i = 0; i < capacity; i++) {
            buff[i] = "";
        }
    }

    /*
     * The destructor for the buffer
     * Called by default when the buffer is out of scope
     *    @returns -- none
     */
    buffer::~buffer() {
        delete(buff);
    }


    /*
     * The function to add an item to the buffer.
     * First acquire the lock for the buffer because only one thread should read or write at a time.
     * If the buffer is not full, proceed with adding the item to the buffer. Otherwise, block itself and the lock is transferred.
     *    @returns -- none
     */
    void buffer::add(std::string item) {

        /* Acquire the lock for critical section */
        std::unique_lock<std::mutex>  pcULock(pcLock); // Mutex wrapper to allow use with condition variables

        assert(count >= 0 && count <= capacity); // Error if the count is greater than capacity or less than 0 at this point
        
        while ( count >= capacity ) {
            /* Wait until the buffer is not full */
            notFull.wait(pcULock);
        }

        /** CRITICAL SECTION **/
        /* Insert the item at the tail end of the buffer */
        buff[tail] = item; // Assign the tail spot to the new item
        tail = (tail + 1) % capacity; // Adjust the tail
        ++count; // Increment the count

        /* Wake up a consumer */
        notEmpty.notify_one( );

        /** END OF CRITICAL SECTION **/
        /* Release the lock */
        pcULock.unlock( );
        return;
    }

    /*
     * The function to remove an item from the buffer.
     * First acquire the lock for the buffer because only one thread should read or write at a time.
     * If the buffer is not empty, proceed with removing the item from the buffer. Otherwise, block itself and the lock is transferred.
     *    @returns -- item: The string located at the head of the buffer
     */
    std::string buffer::remove(void) {
        
        /* Acquire the lock for critical section */
        std::unique_lock<std::mutex>  pcULock(pcLock); // Mutex wrapper to allow use with condition variables

        assert(count >= 0 && count <= capacity); // Error if the count is greater than capacity or less than 0 at this point
        
        while ( count <= 0 ) {
            /* Wait until the buffer is not empty */
            notEmpty.wait(pcULock);
        }
        
        /** CRITICAL SECTION **/
        /* Delete an item at the head end of the buffer */
        std::string item = buff[head]; // Get the item at head
        buff[head] = ""; // Replace with the empty string
        head = (head + 1) % capacity; // Adjust the head
        --count; // Decrement the count

        //assert( item != "" ); // Error if the removed item was the empty string
 
        /* Wake up a producer */
        notFull.notify_one();
        
        /** END OF CRITICAL SECTION **/
        /* Release the lock for critical section */
        pcULock.unlock();

        return item;
    }

}