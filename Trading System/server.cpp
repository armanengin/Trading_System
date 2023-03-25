#include "riskServerLogic.hpp"

//#define MAX 80
#define PORT 8080
#define SA struct sockaddr

//GLOBALS
unordered_map<int, int> messageTypeByPayloadSize{{35,1}, {10,2}, {18, 3}, {34, 4}}; //payload sized mapped with corresponding message types

//FUNCTIONS
/*
description: this function processes the order comes from client by using RiskServerLogic
params: socket id, header of client's order, buffer to store client's order, buyThreshold and selThreshold which are required to chech if new order should be accepted
return type: no return type, however, it sends message to client if the order accepted or not
*/
void processMessage(int sd, Header header, char* buffer, uint64_t buyThreshold, uint64_t sellThreshold){
    int payloadSize = header.payloadSize;
    if(messageTypeByPayloadSize.find(payloadSize) == messageTypeByPayloadSize.end()) return; //if payload size is something unexpected, exit
    OrderResponse response;
    char* messageToClient;
    Header clientHeader;
    read(sd, buffer, payloadSize);
    switch(messageTypeByPayloadSize[payloadSize]){
        case 1:
            NewOrder newOrder;
            memcpy(&newOrder, buffer, payloadSize);
            response = addNewOrder(sd, newOrder, buyThreshold, sellThreshold);
            messageToClient = new char[28];
            clientHeader.payloadSize = 12;
            clientHeader.sequenceNumber = header.sequenceNumber + 1;
            clientHeader.timestamp = chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
            memcpy(messageToClient, &clientHeader, 16);
            memcpy(messageToClient + 16, &response, 12);
            send(sd, messageToClient, 28, 0);
            delete[] messageToClient;//deallocate messageToClient
            break;
        case 2:
            DeleteOrder deleteOrder;
            memcpy(&deleteOrder, buffer+16, payloadSize);
            deleteGivenOrder(deleteOrder);
            break;
        case 3:
            ModifyOrderQuantity modifyOrderQuantity;
            memcpy(&modifyOrderQuantity, buffer+16, payloadSize);
            response = modifyGivenOrderQuantity(modifyOrderQuantity, buyThreshold, sellThreshold);
            messageToClient = new char[28];
            clientHeader.payloadSize = 12;
            clientHeader.sequenceNumber = header.sequenceNumber + 1;
            clientHeader.timestamp = chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
            memcpy(messageToClient, &clientHeader, 16);
            memcpy(messageToClient + 16, &response, 12);
            send(sd, messageToClient, 28, 0);
            delete[] messageToClient;//deallocate messageToClient
            break;
        case 4:
            Trade trade;
            memcpy(&trade, buffer+16, payloadSize);
            makeTrade(trade);
            break;
    }
}
   
//MAIN
int main(int argc , char *argv[]){  
    int opt = 1;  
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;  
    int max_sd;  
    struct sockaddr_in address;  
         
    char buffer[1025];  //data buffer of 1K

    //initial values
    int buyThreshold = 30; 
    int sellThreshold = 20;

    if(argc>=2){
        buyThreshold =  stoi(argv[1]);//init buy threshold with first argument
    }
    if(argc>=3){
        sellThreshold = stoi(argv[2]);//init sell threshold with second argument
    }
        
         
    //set of socket descriptors 
    fd_set readfds;   
         
    //initialise all client_socket[] to 0 so not checked 
    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }  
         
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
     
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
     
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
         
    //bind the socket to localhost port 8888 
    if (::bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0){  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
         
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
         
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  
         
    while(1)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
     
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
             
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
     
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        } 

             
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
             
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
                
            //add new socket to array of sockets 
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                         
                    break;  
                }  
            }


        }  
             
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++){  
            sd = client_socket[i];  
                 
            if (FD_ISSET( sd , &readfds)){  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read(sd , buffer, 16)) <= 0){  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);  
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Handle disconnected user
                    deleteUser(sd);

                    //Close the socket and mark as 0 in list for reuse 
                    close(sd);  
                    client_socket[i] = 0;  
                }  
                     
                else{
                    Header header;
                    memcpy(&header, buffer, 16);

                    processMessage(sd, header, buffer, buyThreshold, sellThreshold); //Handle the incoming messages from clients
                    cout<<"\n";
                }

            }  
        }  
    }  
         
    return 0;  
}