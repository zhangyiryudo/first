#include <iostream>
#include <tbb/concurrent_hash_map.h>

int main() {
    std::cout << "--tbb concurrent_hash_map examples!" << std::endl;
    tbb::concurrent_hash_map<int, std::string> myMap;
    
    // Example 1: Insert elements
    myMap.insert({1, "Hello"});
    myMap.insert({2, "World"});
    myMap.insert({3, "TBB"});
    std::cout << "\n1. After inserting 3 elements:" << std::endl;
    std::cout << "   Map size: " << myMap.size() << std::endl;
    
    // Example 2: Access using find() with accessor
    tbb::concurrent_hash_map<int, std::string>::accessor acc;
    if (myMap.find(acc, 1)) {
        std::cout << "\n2. Found key 1: " << acc->second << std::endl;
    }
    acc.release();
    
    // Example 3: Iterate through the map
    std::cout << "\n3. Iterating through all elements:" << std::endl;
    for (auto it = myMap.begin(); it != myMap.end(); ++it) {
        std::cout << "   Key: " << it->first << ", Value: " << it->second << std::endl;
    }
    
    // Example 4: Erase an element
    myMap.erase(2);
    std::cout << "\n4. After erasing key 2, map size: " << myMap.size() << std::endl;
    
    // Example 5: Insert or assign using mutator
    tbb::concurrent_hash_map<int, std::string>::accessor mut_acc;
    if (myMap.insert(mut_acc, std::make_pair(4, "Parallel"))) {
        std::cout << "\n5. Inserted new key 4: " << mut_acc->second << std::endl;
    }
    mut_acc.release();
    
    // Example 6: Check if key exists
    if (myMap.count(1) > 0) {
        std::cout << "\n6. Key 1 exists in the map" << std::endl;
    }
    
    // Example 7: Clear the map
    myMap.clear();
    std::cout << "\n7. After clear(), map size: " << myMap.size() << std::endl;
    
    return 0;
}