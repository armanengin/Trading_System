#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include <arpa/inet.h>
#include "messageStructs.hpp"
#include <unordered_map>
#include <vector>
using namespace std;

//GLOBALS
unordered_map<uint64_t, NewOrder> orders; //orderIds map with orders
unordered_map<uint64_t, pair<uint64_t, uint64_t>> buyAndSellsForFinancialInstrument; //Financial instrument ids mapped with total buy quantites and total sell quantities corresponding to that financial instrument
unordered_map<uint64_t, int64_t> netPosForFinancialInstrument; //Financial instrument ids mapped with netPos corresponding to that financial instrument
unordered_map<int, vector<uint64_t>> ordersOfUsers;//a socket ID matched with that client's order IDs

//FUNCTIONS
/*
description: this function checks if new order can be added according to the rules, if order accepted it adds the order
params: socket id,  order, buyThreshold and selThreshold which are required to chech if new order should be accepted
return type: OrderResponse -> to inform client about thier order
*/
OrderResponse addNewOrder(int sd, NewOrder order, uint64_t buyThreshold, uint64_t sellThreshold){
    if(buyAndSellsForFinancialInstrument.find(order.listingId) == buyAndSellsForFinancialInstrument.end()){
        buyAndSellsForFinancialInstrument[order.listingId] = {0, 0}; //if this is the first appearence of this financial instrument, init bu and sell wuantities as 0
    }
    bool accepted = 0; //initially order is not accepted
    uint64_t listingId = order.listingId;
    if(order.side == 'B'){
        int hypotheticalWorstBuy = max(buyAndSellsForFinancialInstrument[listingId].first + order.orderQuantity, order.orderQuantity + buyAndSellsForFinancialInstrument[listingId].first + netPosForFinancialInstrument[listingId]);//calculate hypothetical worst buy from the formula
        if(buyThreshold >= hypotheticalWorstBuy){
            accepted = 1; //order can be accepted
            buyAndSellsForFinancialInstrument[order.listingId].first += order.orderQuantity;//increment total bough quantity for specific instrument
        }
    }else if(order.side == 'S'){
        int hypotheticalWorstSell = max(buyAndSellsForFinancialInstrument[listingId].second + order.orderQuantity, order.orderQuantity + buyAndSellsForFinancialInstrument[listingId].second - netPosForFinancialInstrument[listingId]);//calculate hypothetical worst sell from the formula
        if(sellThreshold >= hypotheticalWorstSell){
            accepted = 1; //order can be accepted
            buyAndSellsForFinancialInstrument[listingId].second += order.orderQuantity;//increment total bough quantity for specific instrument
        }
    }
    if(accepted){
        orders[order.orderId] = order;//add new order to orders
        ordersOfUsers[sd].push_back(order.orderId);//add new order to specific user
    }
    OrderResponse orderResponse;
    orderResponse.messageType = 5;
    orderResponse.orderId = order.orderId;
    orderResponse.status = accepted ? OrderResponse::Status::ACCEPTED : OrderResponse::Status::REJECTED;
    
    //provide interface
    cout << "*******************************NEW ORDER REQUEST*******************************" << endl;
    cout << "order id: " << order.orderId << endl;
    cout << "order quantity: " << order.orderQuantity << endl;
    cout << "order price: " << order.orderPrice << endl;
    cout << "financial instrument id: " << order.listingId << endl;
    cout << "order is " << (accepted ? "ACCEPTED" : "REJECTED") << endl;
    cout << "*********************************************************************************" << endl;

    return orderResponse;
}

/*
description: this function delte the given order
params: DeleteOrder
return type: no return type
*/
void deleteGivenOrder(DeleteOrder order){
    uint64_t orderId = order.orderId;
    if(orders.find(orderId) == orders.end()) return;//if the order that will be deleted is not exist, exit from the function
    if(orders[orderId].side == 'B'){  
        buyAndSellsForFinancialInstrument[orders[orderId].listingId].first -= orders[orderId].orderQuantity;//decrement total bough quantity for specific instrument
    }else if(orders[orderId].side == 'S'){
            buyAndSellsForFinancialInstrument[orders[orderId].listingId].second -= orders[orderId].orderQuantity;//decrement total bough quantity for specific instrument
    }
    //provide interface
    cout << "*******************************DELETE ORDER REQUEST*******************************\n";
    cout << "deleted order id: " << order.orderId << "\n";
    cout << "deleted order's quantity: " << orders[order.orderId].orderQuantity << "\n";
    cout << "deleted order's price: " << orders[order.orderId].orderPrice << "\n";
    cout << "delted order's financial instrument id: " << orders[order.orderId].listingId << "\n";
    cout << "*************************************************************************************\n";

    orders.erase(orderId);//delete order from the map
}

/*
description: this function delte the given order
params: NewOrder
return type: no return type
*/
void deleteGivenOrder(NewOrder order){
    uint64_t orderId = order.orderId;
    if(orders.find(orderId) == orders.end()) return;//if the order that will be deleted is not exist, exit from the function
    if(orders[orderId].side == 'B'){  
        buyAndSellsForFinancialInstrument[orders[orderId].listingId].first -= orders[orderId].orderQuantity;//decrement total bough quantity for specific instrument
    }else if(orders[orderId].side == 'S'){
            buyAndSellsForFinancialInstrument[orders[orderId].listingId].second -= orders[orderId].orderQuantity;//decrement total bough quantity for specific instrument
    }
    orders.erase(orderId);//delete order from the map
}

/*
description: this function checks if the quantity of already given order can be changed, if it is accepted it modifies the quantity of that order
params: ModifyOrderQuantity -> the new quantity and order id given, 2 uint64_t variables for buy threshold and sell threshold to check if nodify request can be accepted
return type: OrderResponse -> to inform client about thier order
*/
OrderResponse modifyGivenOrderQuantity(ModifyOrderQuantity order, uint64_t buyThreshold, uint64_t sellThreshold){
    bool accepted = false; //initially modify order is not accepted
    uint64_t orderId = order.orderId;
    uint64_t listingId = orders[orderId].listingId;
    if(orders.find(orderId) != orders.end()){//if order is not found skip here
        if(orders[orderId].side == 'B'){
            int differenceFromPrevQuant = order.newQuantity - orders[orderId].orderQuantity;
            int hypotheticalWorstBuy = max(buyAndSellsForFinancialInstrument[listingId].first + differenceFromPrevQuant, differenceFromPrevQuant + buyAndSellsForFinancialInstrument[listingId].first + netPosForFinancialInstrument[listingId]);//calculate hypothetical worst buy from the formula
            if(buyThreshold >= hypotheticalWorstBuy){
                accepted = true; //order can be accepted
                buyAndSellsForFinancialInstrument[orders[orderId].listingId].first += differenceFromPrevQuant;//add difference to the total quantity for the financial instrument
                orders[orderId].orderQuantity += differenceFromPrevQuant; //update order with new quantity
            }
        }else if(orders[orderId].side == 'S'){
            int differenceFromPrevQuant = order.newQuantity - orders[orderId].orderQuantity;
            int hypotheticalWorstSell = max(buyAndSellsForFinancialInstrument[listingId].second + differenceFromPrevQuant, differenceFromPrevQuant + buyAndSellsForFinancialInstrument[listingId].second - netPosForFinancialInstrument[listingId]);//calculate hypothetical worst sell from the formula
            if(sellThreshold >= hypotheticalWorstSell){
                accepted = true; //order can be accepted
                buyAndSellsForFinancialInstrument[listingId].second += differenceFromPrevQuant;//add difference to the total quantity for the financial instrument
                orders[orderId].orderQuantity += differenceFromPrevQuant; //update order with new quantity
            }
        }
    }
    OrderResponse orderResponse;
    orderResponse.messageType = 5;
    orderResponse.orderId = order.orderId;
    orderResponse.status = accepted ? OrderResponse::Status::ACCEPTED : OrderResponse::Status::REJECTED;

    //provide interface
    cout << "*******************************Modify ORDER REQUEST*******************************" << endl;
    cout << "order id: " << order.orderId << endl;
    cout << "new order quantity: " << order.newQuantity << endl;
    cout << "order price: " << orders[orderId].orderPrice << endl;
    cout << "financial instrument id: " << orders[orderId].listingId << endl;
    cout << "order is " << (accepted ? "ACCEPTED" : "REJECTED") << endl;
    cout << "***********************************************************************************" << endl;

    return orderResponse;
}

/*
description: this function makes trade and change the netPosition of a financial instrument
params: Trade trade -> includes details about trade
return type: no return type
*/
void makeTrade(Trade trade){
    uint64_t orderId = trade.tradeId;
    if(orders.find(orderId) == orders.end()) return; // if order is not exist exit from function
    uint64_t listingId = orders[orderId].listingId;
    netPosForFinancialInstrument[listingId] += trade.tradeQuantity; //update netPos for given trade quantity for the financial instrument

    //provide interface
    cout << "*******************************TRADE REQUEST*******************************" << endl;
    cout << "trade id: " << trade.tradeId << endl;
    cout << "trade quantity: " << trade.tradeQuantity << endl;
    cout << "order price: " << trade.tradePrice << endl;
    cout << "financial instrument id: " << trade.listingId << endl;
    cout << "***************************************************************************" << endl;
}

/*
description: this function deletes all orders of a user -> when client disconnected this function will be called
params: socket id to clarify the client
return type: no return type
*/
void deleteUser(uint64_t sd){
    vector<uint64_t> userOrders = ordersOfUsers[sd];
    uint64_t i = 0;
    cout << "*******************************DELETE USER*******************************" << endl;
    cout << "Following orders are deleted: " << endl;
    while(i < userOrders.size()){
        if(orders.find(userOrders[i]) == orders.end()){//if order already deleted, skip this order
            swap(userOrders[i], userOrders.back());
            userOrders.pop_back();
            continue;
        }else{
            cout << "order id: " << userOrders[i] << endl;
            deleteGivenOrder(orders[userOrders[i]]);
            swap(userOrders[i], userOrders.back());
            userOrders.pop_back();
        }
    }
    cout << "**************************************************************************" << endl;
}