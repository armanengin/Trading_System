#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include "messageStructs.hpp"
#include <unordered_map>
#include <chrono>
using namespace std;

//#define MAX 80
#define PORT 8080
#define SA struct sockaddr

//GLOBALS
int id = 1;
int sequenceNum = 1;
unordered_map<int, int> messageTypesSizes {{1, 51}, {2, 26}, {3, 34}, {4, 50}};// unordered map the maps message types to their relative sizes in bytes

//FUNCTIONS
/*
description: for given message type a message created and returned
params: an integer corresponds to the message type
return type: char* returned which stores the message
*/
char* createMessage(int messageType){
    char* message = NULL;
    switch(messageType){//1(newOrder), 2(deleteOrder), 3(modifyOrder), 4(trade)
        Header header;
        NewOrder newOrder;
        case 1 : {
            //init header
            header.version = 1;
            header.payloadSize = 35;
            header.sequenceNumber = sequenceNum++;
            header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();;
            //create newOrder
            newOrder.messageType = messageType;
            cout << "******************************ENTER******************************" << endl;
            cout << "Enter financial instrument ID:" << endl; cin >> newOrder.listingId;
            newOrder.orderId = id++; //init id and increment it by 1 for next initilization
            cout << "Enter order quantity:" << endl; cin >> newOrder.orderQuantity;
            cout << "Enter order price:" << endl; cin >> newOrder.orderPrice;
            char side = ' ';
            cout << "Enter B to buy, S to sell:" << endl; cin >> side;
            while(side != 'B' && side != 'S'){//until valid side is entered, ask user to enter again
                cout << "Wrong character entered, please try again." << endl;
                cout << "Enter B to buy, S to sell:" << endl; cin >> side;
            }
            newOrder.side = side;
            message = new char[51];
            memcpy(message, &header, 16);
            memcpy(message + 16, &newOrder, 35);
            break;
        }
        case 2 : {
            //init header
            header.version = 1;
            header.payloadSize = 10;
            header.sequenceNumber = sequenceNum++;
            header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();;
            //create newOrder
            DeleteOrder deleteOrder;
            deleteOrder.messageType = messageType;
            cout << "Enter order ID that you want to delete: \n"; cin >> deleteOrder.orderId;
            message = new char[26];
            memcpy(message, &header, 16);
            memcpy(message + 16, &deleteOrder, 10);
            break;
        }
        case 3 : {
            //init header
            header.version = 1;
            header.payloadSize = 18;
            header.sequenceNumber = sequenceNum++;
            header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();;
            //create newOrder
            ModifyOrderQuantity modifyOrder;
            newOrder.messageType = messageType;
            cout << "Enter order ID that you want to modify: \n"; cin >> modifyOrder.orderId;
            cout << "Enter new order quantity:\n"; cin >> modifyOrder.newQuantity;
            message = new char[34];
            memcpy(message, &header, 16);
            memcpy(message + 16, &modifyOrder, 18);
        }
        case 4 : {
            //init header
            header.version = 1;
            header.payloadSize = 34;
            header.sequenceNumber = sequenceNum++;
            header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();;
            //create newOrder
            Trade trade;
            trade.messageType = messageType;
            cout << "Enter financial instrument ID:" << endl; cin >> trade.listingId;
            cout << "Enter trade ID:" << endl; cin >> trade.tradeId; //init id and increment it by 1 for next initilization
            cout << "Enter trade Quantity:" << endl; cin >> trade.tradeQuantity;
            cout << "Enter trade price:" << endl; cin >> trade.tradePrice;
            message = new char[50];
            memcpy(message, &header, 16);
            memcpy(message + 16, &trade, 34);
            break;
        }
        default :
            cout << "Invalid message type entered! Please try again." << endl;
    }
    return message;
}

//MAIN
int main(int argc, char const *argv[]){
    struct sockaddr_in servAddr;
    int sock = 0;
    char buffer[1025];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Socket creation error \n");
        return -1;
    }
   
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
       
    if(inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr)<=0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0){
        printf("\nConnection Failed\n");
        return -1;
    }

    int messageType = -1; //init with -1
    
    //iterates until client disconnects.
    while(1){
        cout << "Enter 1 for new order, 2 for delete an order, 3 to modify an order, 4 to trade, 0 to disconnect: \n";
        do{
            cin >> messageType;
            if(messageType >= 0 && messageType <= 4) break;
            cout << "Wrong message type Entered! Please enter again from 1 to 4 inclusive.";
        }while(messageType < 0 || messageType > 4);
        if(messageType == 0){
            break; //disconnect
        }
        else{
            char* message = NULL;
            while(!message){//until a valid message is created keep calling the createMessage function
                message = createMessage(messageType);
            }
            send(sock, message, messageTypesSizes[messageType], 0); //Sends the message to the server
            delete[] message; //deallocate message after sending it
            // If new order or order modification requested, risk server checks them for acceptance
            if(messageType == 1 || messageType == 3){
                //No response received from server
                if (read(sock, buffer, 16) <= 0){
                    std::cerr<<"Server disconnected";
                    break;
                }

                Header header;
                memcpy(&header, buffer, 16); //init header from response's header

                read(sock, buffer, header.payloadSize);// read the response message
                OrderResponse response;
                memcpy(&response, buffer, header.payloadSize);
                cout<<"**********************************RESPONSE FROM RISK SERVER**********************************\n" << endl;
                cout<<"Risk server's response for the order with ID " << response.orderId << " is: ";
                if(response.status == OrderResponse::Status::ACCEPTED)
                    cout<<"ACCEPTED!\n" << endl;
                else
                    cout<<"REJECTED!\n" << endl;
                cout<<"******************************************************************************************************\n";
                cout<<std::endl;
            }
        }
    }
    return 0;
}