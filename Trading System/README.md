# Trading system(C++)
- A trading system that clients can request bids and asks. Server evaluates bid and asks according to an algorithm and rejects or approves the requests.
- **Concept/Technologies:** Client server communication by using TCP, sockets

The solution implemented by using c++'s standart containers
1. What could be done better
    on testing lots of time spent for including google test library. Since I had limited time instead of using google test library I could have directly focusing on writing unit testing without using a library. However, provideed basic UI with couts to track code's relaibility.

2. How to run:
* ./server argument1 argument2
* ./client

3. Used Datastructures
* 4 unordered_map used -> the use cases are following:
* In order to track buy and sell quantities for every instrument -> unordered_map<uint64_t, pair<uint64_t, uint64_t>> used
* In order to track net position for every instrument -> unordered_map<uint64_t, int64_t> used
* In order to check if given id corresponds to an order in constant time -> unordered_map<uint64_t, NewOrder> used
* In order to track which user has which orders -> unordered_map<int, vector<uint64_t>>
    * by using this unordered_map, vector of order_ids can be attained. However, while poping an order_id from the vector, in order to decrease the time complexity following has been done. Before popping from the current index, current index's value and the one at the back has been swaped, so that popping from vector now takes O(1) time( which would take O(n) without improvement)
    These details especially important, since in reality every day millions, even billions of orders could be processed, therefore, efficincy of the algorithm is vital, especially in high frequency trading industry, since it is pretty competetive.
