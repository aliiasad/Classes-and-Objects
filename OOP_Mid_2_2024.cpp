# include <iostream>
#include <cstring>
using namespace std;

class MyList    {
    private:
        char** arr;
        int size;
    public:
        MyList();
        MyList(char**, int);
        MyList(const MyList&);
        ~MyList();

        MyList operator+(const MyList&);
        MyList& operator=(const MyList&);
        friend MyList operator+(char*, const MyList&);
        friend ostream& operator<<(ostream&, const MyList&);
};

MyList :: MyList()  {
    size = 0;
    arr = new char* [size];
    for (int i = 0; i < size; i++)  {
        arr[i] = "";
    }
}

MyList :: MyList(const MyList& dummy)   {
    size = dummy.size;
    arr = new char* [size];
    for (int i = 0; i < size; i++)  {
        arr[i] = new char [strlen(dummy.arr[i]) + 1];
        strcpy(arr[i], dummy.arr[i]);
    }
}

MyList :: MyList(char** list, int sz)   {
    size = sz;
    arr = new char* [sz];
    for (int i = 0; i < sz; i++)  {
        arr[i] = new char [strlen(list[i]) + 1];
        strcpy(arr[i], list[i]);
    }
}

MyList& MyList :: operator=(const MyList& dummy)    {
    if (this == &dummy) return *this;

    for (int i = 0; i < size; i++)  
        delete[] arr[i];
    delete[] arr;

    size = dummy.size;
    arr = new char* [size];
    for (int i = 0; i < size; i++)  {
        arr[i] = new char [strlen(dummy.arr[i]) + 1];
        strcpy(arr[i], dummy.arr[i]);
    }

    return *this;
}

MyList MyList :: operator+(const MyList& dummy) {
    int newSize = size + dummy.size;
    char** newList = new char* [newSize];
    for (int i = 0; i < size; i++)   {
        newList[i] = new char [strlen(arr[i]) + 1];
        strcpy(newList[i], arr[i]);
    }
    for (int i = 0; i < dummy.size; i++)   {
        newList[size + i] = new char [strlen(dummy.arr[i]) + 1];
        strcpy(newList[size + i], dummy.arr[i]);
    }

    MyList result(newList, newSize);
    for (int i = 0; i < newSize; i++)  
        delete[] newList[i];
    delete[] newList;

    return result;
}

MyList operator+(char* list, const MyList& dummy)  {
    int newSize = 1 + dummy.size;
    char** newList = new char* [newSize];
    for (int i = 0; i < dummy.size; i++)   {
        newList[i] = new char [strlen(dummy.arr[i]) + 1];
        strcpy(newList[i], dummy.arr[i]);
    }
    newList[newSize - 1] = new char [strlen(list) + 1];
    strcpy(newList[newSize - 1], list);

    MyList result(newList, newSize);
    for (int i = 0; i < newSize; i++)  
        delete[] newList[i];
    delete[] newList;

    return result;
}

ostream& operator<<(ostream& out, const MyList& dummy)    {
    out << "[";
    for (int i = 0; i < dummy.size; i++)    {
        if (i == dummy.size - 1)    out << dummy.arr[i];
        else {
            out << dummy.arr[i] << ", ";
        }
    }
    out << "]";
    return out;
}

MyList :: ~MyList() {
    for (int i = 0; i < size; i++)  
        delete[] arr[i];
    delete[] arr;
}

int main()  {
    MyList list1;
    cout << "list1 = " << list1 << endl;

    list1 = "Apple" + list1;
    cout << "list1 = " << list1 << endl;

    list1 = "Banana" + list1;
    cout << "list1 = " << list1 << endl;

    list1 = "Peach" + list1;
    cout << "list1 = " << list1 << endl;

    MyList list2;
    cout << "list2 = " << list2 << endl;

    list2 = "I love Pakistan" + list2;
    cout << "list2 = " << list2 << endl;

    list2 = "Happy Programming" + list2;
    cout << "list2 = " << list2 << endl;

    MyList list3 = list1 + list2;
    cout << "List 3 = " << list3 << endl;
    return 0;
}
