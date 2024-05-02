A README file that describes 
(1) what you got working, 
(2) a description of your critical sections, 
(3) the buffer size that gave optimal performance for 30 or more files, 
(4) tell me in which stage you would add an additional thread to improve performance and why you chose that stage, and 
(5) Any bugs in your code.

Authors: Meghan Grayson and Vaishnavi Karaguppi

(1) What we got working:

The program works fully as described by the project description, but it does not do the bonus requirement of performing a recursive search within the subdirectories.

(2) A description of the critical sections:

The critical sections are located in the buffer.cpp, or the implementation of the bounded buffer class. There is a critical section inside the add and remove functions. The values of count, tail, head, and buffer contents can only be read or changed within the critical section protected by the mutex. Threads must acquire the lock upon entering the add or remove functions since they must immediately assert the value of count is valid. 

During the add function, the thread may invoke wait function on notFull if the count is at capacity. The wait function blocks the thread and effectively releases the mutex to another thread attempting to acquire it. Once the waiting thread receives notify() from a consumer, it can reacquire the lock and continue its execution. The thread reassesses the count in the while loop and leaves if possible. Since it must alter the contents of the buffer, the value of the tail, and the value of the count, these are all contained within the critical section to protect against race conditions known as a "time of check time of use" race, where values can be incorrectly updated. Lastly, the thread sends a notify() to a consumer that may be waiting on notEmpty. This should also be protected by the mutex just in case a consumer arrives to wait() after the producer has sent notify(), causing the consumer to not be notified and continue to wait.

The add critical section looks like below:

assert(count >= 0 && count <= capacity); // Error if the count is greater than capacity or less than 0 at this point
while ( count >= capacity ) {
	/* Wait until the buffer is not full */
	notFull.wait(pcULock);
}
buff[tail] = item; // Assign the tail spot to the new item
tail = (tail + 1) % capacity; // Adjust the tail
++count; // Increment the count
/* Wake up a consumer */
notEmpty.notify_one( ); // Wake up a consumer

During the remove function, the thread may invoke wait function on notEmpty if the count is at or below 0. Similar to the add function, the wait blocks the thread and effectively releases the mutex to another thread attempting to acquire it. Once the waiting thread receives notify() from a producer, it can reacquire the lock and continue its execution. The thread reassesses the count in the while loop and leaves if possible. Since it must alter the contents of the buffer, the value of the head, and the value of the count, these are all protected by the mutex in the critical section. Lastly, the thread can send a notify() to a waiting producer on notFull. This should also be protected by the mutex just in case a producer arrives to wait() after the producer has sent notify(), causing the producer to not be notified.

The remove critical section looks like below:

assert(count >= 0 && count <= capacity); // Error if the count is greater than capacity or less than 0 at this point      
while ( count <= 0 ) {
	/* Wait until the buffer is not empty */
	notEmpty.wait(pcULock);
}
/* Delete an item at the head end of the buffer */
std::string item = buff[head]; // Get the item at head
buff[head] = ""; // Replace with the empty string
head = (head + 1) % capacity; // Adjust the head
--count; // Decrement the count
/* Wake up a producer */
notFull.notify_one();

(3) The buffer size that gave optimal performance for 30 or more files:

By using the time system function, a buffer size of around 50 gave the optimal performance.

(4) Which stage would we add an additional thread to improve performance and why:

The fourth stage has the worst time complexity as it's performing a search through every line contained in buff3, so this would be the best stage to add an additional thread to improve performance. Other stages are linear time complexity since they are generally just adding sequentially to a buffer. In stage 4, the search time for one line is complexity O(NxM) where N is the length of the entire line and M is the length of the substring. This multiplied by the total number of lines generated from stage 3 makes for an inefficient search time, becoming a bottleneck for the program. Performing the search on items from buff3 in parallel would improve overall performance the most out of all stages. 

(5) Any bugs:

There are no known bugs in the code.

