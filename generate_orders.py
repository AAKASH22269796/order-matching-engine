import json 
import random
def generate_orders(num_orders=50,num_users=8,base_price=50,price_spread=10):
    orders = []
    for i in range(1,num_orders+1):
        order={
            "orderId": i,
            "userId": random.randint(1,num_users),
            "price": round(random.uniform(base_price-price_spread, base_price+price_spread),2),
            "quantity": random.randint(10,100),
            "timestamp": i,
            "isBuy": random.choice([True, False])
        }
        orders.append(order)
    return orders

if __name__ == "__main__":
    orders = generate_orders()
    with open("orders.json", "w") as f:
        json.dump(orders, f, indent=2)
    print(f"Generated {len(orders)} orders and saved to orders.json")