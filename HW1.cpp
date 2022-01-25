#include <vector>
#include <iostream>
#include <algorithm>
#include <thread>
#include <filesystem>
#include <fstream>
#include <time.h>
using namespace std;

const int MAX = 1e8;
const int NUM_THREADS = 8;
const int NUM_TOP_PRIMES = 10;
const string OUTPUT_FILENAME = "primes.txt";

// create a large global array storing isPrime[num] = false if we have discovered this number to be composite,
// true otherwise
bool isPrime[MAX + 1];

void sieve(int threadIdx){
    // each thread will be responsible for every (NUM_THREADS)th odd number, each next thread staggered from the last
    for(int num = 3 + 2 * threadIdx; num * num <= MAX; num += 2 * NUM_THREADS){
        // this appears to be unsafe as multiple threads are reading and writing from isPrime at the same time.
        // however, since this is a boolean array and at each index we are only concerned about a literal bit of
        // information, we will treat this as alright. even if a thread is under the impression that a number is
        // prime even if we will later discover that it is composite, we will only lose out on efficiency as we
        // will sieve the number. correctness will not be lost

        // if we have already discovered this number is not prime, we can skip it
        if(!isPrime[num]){
            continue;
        }

        for(int mult = num + num; mult <= MAX; mult += num){
            // set each multiple of this number besides itself to be composite. this also appears to be unsafe, but
            // since we are not too concerned with executing everything in a perfect order, this will do
            isPrime[mult] = false;
        }
    }
}

int main(){
    // we will implement a classic prime sieve, but parallelized
    fill(begin(isPrime), end(isPrime), true);

    // although 0, 1, and all even numbers after 2 will not be correctly identified as non-prime after our sieve, we will
    // not be too concerned as long as we avoid checking for these indices in our sieve at the end when making calculations

    // we will time the calculations from start to finish of the spawned threads
    auto startTime = chrono::system_clock::now();
    
    vector<thread*> threads;
    for(int i = 0; i < NUM_THREADS; ++i){
        threads.push_back(new thread(sieve, i));
    }

    for(auto currThread : threads){
        currThread->join();
    }

    auto endTime = chrono::system_clock::now();

    double time = ((chrono::duration<double>)(endTime - startTime)).count();

    // include 2 in our sum and count of primes as we will not check for it in the following for loop
    long long sum = 2;
    int count = 1;
    for(int num = 3; num <= MAX; num += 2){
        if(isPrime[num]){
            sum += num;
            ++count;
        }
    }

    vector<int> topPrimes;
    // start at first odd number and work downward from there
    for(int num = MAX - (MAX % 2 == 0 ? 1 : 0); num >= 3 && size(topPrimes) < NUM_TOP_PRIMES; num -= 2){
        if(isPrime[num]){
            topPrimes.push_back(num);
        }
    }
    // they are in backwards order so reverse them
    reverse(begin(topPrimes), end(topPrimes));

    // get some file output goin
    ofstream out(OUTPUT_FILENAME);

    out << setprecision(3) << fixed << time << " " << count << " " << sum << "\n";
    for(int a : topPrimes){
        out << a << " ";
    }
    out << "\n";

    return 0;
}