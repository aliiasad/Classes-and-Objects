# include <iostream>
using namespace std;

class Set {
    private:
        int* elements;
        int size;
    public:
        Set();
        Set(int*, int);
        Set(const Set&);
        ~Set();

        friend ostream& operator<<(ostream&, const Set&);
        Set& operator=(const Set&);
        Set operator+(const Set&);
        Set operator--(int);
        friend Set operator+(int*, const Set&);
};

Set :: Set()    {
    size = 0;
    elements = new int [size];
    for (int i = 0; i < size; i++)  {
        elements[i] = 0;
    }
}

Set :: Set(int* elem, int size) {
    this->size = size;
    elements = new int [size];
    for (int i = 0; i < size; i++)  {
        elements[i] = elem[i];
    }
}

Set :: Set(const Set& dummy)    {
    size = dummy.size;
    elements = new int [size];
    for (int i = 0; i < size; i++)  {
        elements[i] = dummy.elements[i];
    }
}

ostream& operator<<(ostream& out, const Set& dummy) {
    out << "{";
    for (int i = 0; i < dummy.size; i++)    {
        if (i == dummy.size - 1)    out << dummy.elements[i] << "}";
        else {
            out << dummy.elements[i] << ", ";
        }
    }
    return out;
}

Set& Set :: operator=(const Set& dummy) {
    if (this == &dummy) return *this;

    delete[] elements;

    size = dummy.size;
    elements = new int [size];
    for (int i = 0; i < size; i++)  {
        elements[i] = dummy.elements[i];
    }
    return *this;
}

Set Set :: operator+(const Set& dummy)  {
    int newSize = size + dummy.size;
    int* newArray = new int [newSize];
    int i = 0, j = 0, k = 0;
    while (i < size && j < dummy.size) {
        if (elements[i] < dummy.elements[j]) {
            newArray[k++] = elements[i++];
        }   else if (elements[i] > dummy.elements[j]) {
         newArray[k++] = dummy.elements[j++];
        }   else {
            newArray[k++] = elements[i++];
            j++; // skip duplicate
        }
    }
    while (i < size) {
        newArray[k++] = elements[i++];
    }
    while (j < dummy.size) {
        newArray[k++] = dummy.elements[j++];
    }

    Set result(newArray, k);
    delete[] newArray;
    return result;
}

Set Set :: operator--(int)    {
    Set temp(*this);
    for (int i = 0; i < size; i++)  {
        elements[i]--;
    }
    return temp;
}

Set :: ~Set()   {
    delete[] elements;
}

Set operator+(int* arr, const Set& dummy)    {
    int newSize = 0;
    while (arr[newSize] != -1)  newSize++;
    int* newArray = new int [newSize];
    for (int i = 0; i < newSize; i++)    {
        newArray[i] = arr[i];
    }

    int* final  = new int [newSize + dummy.size];
    int i = 0, j = 0, k = 0;
    while (i < newSize && j < dummy.size) {
        if (newArray[i] < dummy.elements[j]) {
            final[k++] = newArray[i++];
        }   else if (newArray[i] > dummy.elements[j]) {
            final[k++] = dummy.elements[j++];
        }   else {
            final[k++] = newArray[i++];
            j++; // skip duplicate
        }
    }
    while (i < newSize) {
        final[k++] = newArray[i++];
    }
    while (j < dummy.size) {
        final[k++] = dummy.elements[j++];
    }

    Set result(final, k);
    delete[] newArray;
    delete[] final;

    return result;
}

int main()  {
    int arr1[] = {10, 20, 30, 40};
    Set s1(arr1, 4);

    int arr2[] = {5, 15, 55};
    Set s2(arr2, 3);

    cout << "Set 1:\t" << s1 << "Set 2:\t" << s2;

    Set s3;
    s3 = s1 + s2;
    cout << "Set 3:\t" << s3;

    int arr3[] = {1, 2, 100, -1};
    s3 = arr3 + s1;
    cout << "Set 3:\t" << s3;

    cout << s1--;
    cout << s1;
    return 0;
}