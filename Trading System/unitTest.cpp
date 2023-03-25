//#include <gtest/gtest.h>
//can not include google test library
#include "riskServerLogic.hpp"



int main(int argc, char const *argv[]){

    int buyThreshold = 100;
    int sellThreshold = 90;

    NewOrder order1;
    order1.listingId = 1;
    order1.messageType = 1;
    order1.orderId = 1;
    order1.orderPrice = 100000;
    order1.orderQuantity = 40;
    order1.side = 'B';

    NewOrder order2;
    order2.listingId = 1;
    order2.messageType = 1;
    order2.orderId = 2;
    order2.orderPrice = 100000;
    order2.orderQuantity = 25;
    order2.side = 'S';

    NewOrder order3;
    order3.listingId = 1;
    order3.messageType = 1;
    order3.orderId = 3;
    order3.orderPrice = 100000;
    order3.orderQuantity = 40;
    order3.side = 'S';

    NewOrder order4;
    order4.listingId = 1;
    order4.messageType = 1;
    order4.orderId = 4;
    order4.orderPrice = 100000;
    order4.orderQuantity = 48;
    order4.side = 'B';

    NewOrder order5;
    order5.listingId = 2;
    order5.messageType = 1;
    order5.orderId = 5;
    order5.orderPrice = 100000;
    order5.orderQuantity = 75;
    order5.side = 'B';

    NewOrder order6;
    order6.listingId = 2;
    order6.messageType = 1;
    order6.orderId = 6;
    order6.orderPrice = 100000;
    order6.orderQuantity = 85;
    order6.side = 'S';

    unordered_map<uint64_t, NewOrder> orders{{1, order1}, {2, order2}, {3, order3}, {4, order4}, {5, order5}, {6, order6}};
    unordered_map<uint64_t, pair<uint64_t, uint64_t>> buyAndSellsForFinancialInstrument{{1, {88, 65}}, {2, {75, 85}}}; //Financial instrument ids mapped with total buy quantites and total sell quantities corresponding to that financial instrument
    unordered_map<uint64_t, int64_t> netPosForFinancialInstrument{{1, 0}, {2, 0}}; //Financial instrument ids mapped with netPos corresponding to that financial instrument
    unordered_map<int, vector<uint64_t>> ordersOfUsers{{1,{1,2,3,4,5,6}}};//a socket ID matched with that client's order IDs


    NewOrder testOrder1;
    testOrder1.listingId = 1;
    testOrder1.messageType = 1;
    testOrder1.orderId = 7;
    testOrder1.orderPrice = 100000;
    testOrder1.orderQuantity = 26;
    testOrder1.side = 'S';

    Trade trade1;
    trade1.listingId = 2;
    trade1.messageType = 4;
    trade1.tradeId = 1;
    trade1.tradePrice = 100000;
    trade1.tradeQuantity = 25;

    NewOrder testOrder2;
    testOrder1.listingId = 2;
    testOrder1.messageType = 1;
    testOrder1.orderId = 8;
    testOrder1.orderPrice = 100000;
    testOrder1.orderQuantity = 1;
    testOrder1.side = 'B';


    makeTrade(trade1);
    if(addNewOrder(1, testOrder2, 100, 90).status == OrderResponse::Status::REJECTED){
        cout << "both makeTrade and addNewOrder passed the test" << endl;
    }else{
        cout << "either makeTrade failed the test or addNewOrder failed the test" << endl;
    }
}
