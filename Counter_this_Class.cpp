# include <iostream>
using namespace std;

class Counter   {
    private:
        int count;
    public:
        Counter();
        Counter& increment();
        Counter& decrement();
        void display();
};

Counter :: Counter()    {
    count = 0;
}

Counter& Counter :: increment() {
    this->count++; // same as *this.count++ and count++
    return *this;
}

Counter& Counter :: decrement() {
    this->count--; // same as *this.count-- and count--
    return *this;
}

void Counter :: display()   {
    cout << this->count << endl; // same as count and *this.count
}

int main()  {
        Counter c;
        c.increment().increment().increment().decrement().display();
    return 0;
}