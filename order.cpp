#include<bits/stdc++.h>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;
struct Order{
    int orderId;
    int userId;
    bool isBuy;
    double price;
    int quantity;
    int timestamp;
};
struct Trade{
    int buyOrderId;
    int buyUserId;
    int sellOrderId;
    int sellUserId;
    double price;
    int quantity;
    int timestamp;
};
struct compareBid{
    bool operator()(Order const& a, Order const& b){
        if(a.price == b.price){
            return a.timestamp > b.timestamp; // Higher timestamp means more recent
        }
        return a.price < b.price; 
    }
};
struct compareAsk{
    bool operator()(Order const& a, Order const& b){
        if(a.price == b.price){
            return a.timestamp > b.timestamp; // Higher timestamp means more recent
        }
        return a.price > b.price; 
    }
};

priority_queue<Order,vector<Order>,compareBid>bids;
priority_queue<Order,vector<Order>,compareAsk>asks;
vector<Trade>allTrades;

void addOrder(Order order){
    if(order.isBuy){
        bids.push(order);
    }else{
        asks.push(order);
    }
}
void matchOrders(){
    vector<Order>skippedAsks;
    while(!bids.empty()&&!asks.empty()&&bids.top().price>=asks.top().price){
        Order bestBid=bids.top();
        Order bestAsk=asks.top();
        if(bestBid.userId==bestAsk.userId){
            asks.pop();
            skippedAsks.push_back(bestAsk);
            continue;
        }
        else{
            int mini=min(bestBid.quantity,bestAsk.quantity);
            bestBid.quantity-=mini;
            bestAsk.quantity-=mini;

            Trade trade;
            trade.buyOrderId = bestBid.orderId;
            trade.buyUserId = bestBid.userId;
            trade.sellOrderId = bestAsk.orderId;
            trade.sellUserId = bestAsk.userId;
            trade.quantity = mini;
            
            if(bestBid.timestamp < bestAsk.timestamp){
                trade.price = bestBid.price;
                trade.timestamp = bestAsk.timestamp;
            }
            else{
                trade.price = bestAsk.price;
                trade.timestamp = bestBid.timestamp;
            }
            allTrades.push_back(trade);

            if(bestBid.quantity==0&&bestAsk.quantity==0){
                bids.pop();
                asks.pop();
            }
            else{
                bids.pop();
                asks.pop();
                if(bestBid.quantity==0){
                    asks.push(bestAsk);
                }
                else{
                    bids.push(bestBid);
                }
            }
        }
        if(!bids.empty()&&skippedAsks.size()>0&&bids.top().userId!=skippedAsks.back().userId){
            for(Order& o : skippedAsks){
                asks.push(o);
            }
            skippedAsks.clear();
        }
    }

    for(Order& o : skippedAsks){
        asks.push(o);
    }
}

int main(){
    ifstream inputFile("orders.json");
    json data;
    inputFile >> data;
    vector<Order>allOrders;
    for(auto& item : data){
        Order o;
        o.orderId = item["orderId"];
        o.userId = item["userId"];
        o.isBuy = item["isBuy"];
        o.price = item["price"];
        o.quantity = item["quantity"];
        o.timestamp = item["timestamp"];
        allOrders.push_back(o);
    }
    sort(allOrders.begin(),allOrders.end(),[](Order const& a, Order const& b){
        return a.timestamp < b.timestamp;
    });
    for(Order& o : allOrders){
        addOrder(o);
        matchOrders();
    }

    json tradesJson = json::array();
    for (Trade& t : allTrades) {
        tradesJson.push_back({
            {"buyOrderId", t.buyOrderId},
            {"buyUserId", t.buyUserId},
            {"sellOrderId", t.sellOrderId},
            {"sellUserId", t.sellUserId},
            {"price", t.price},
            {"quantity", t.quantity},
            {"timestamp", t.timestamp}
        });
    }
    ofstream outFile("trades.json");
    outFile << tradesJson.dump(4);
    outFile.close();
    
    json orderbookJson;
    orderbookJson["bids"] = json::array();
    orderbookJson["asks"] = json::array();
    cout << "\n----- REMAINING BIDS -----\n";
    while (!bids.empty()) {
        Order o = bids.top(); bids.pop();
        cout << "Order " << o.orderId << " (User " << o.userId << "): "
        << o.quantity << " @ Rs" << o.price << "\n";
        orderbookJson["bids"].push_back({
            {"orderId", o.orderId},
            {"userId", o.userId},
            {"price", o.price},
            {"quantity", o.quantity},
            {"timestamp", o.timestamp}
        });
    }
    while (!asks.empty()) {
        Order o = asks.top(); asks.pop();
        cout << "Order " << o.orderId << " (User " << o.userId << "): "
        << o.quantity << " @ Rs" << o.price << "\n";
        orderbookJson["asks"].push_back({
            {"orderId", o.orderId},
            {"userId", o.userId},
            {"price", o.price},
            {"quantity", o.quantity},
            {"timestamp", o.timestamp}
        });
    }

    ofstream bookFile("orderbook.json");
    bookFile << orderbookJson.dump(4);
    bookFile.close();

}






