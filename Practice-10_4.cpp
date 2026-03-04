#include <iostream>
#include <cstdlib>
using namespace std;

void mystrcpy(const char src[], char dest[]);
int mystrlen(const char str[]);

void mystrcpy(const char src[], char dest[])    {
    int i = 0;
    while (src[i] != '\0')  {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return;
}

int mystrlen(const char str[])    {
    int i = 0;
    while (str[i] != '\0')  {
        i++;
    }
    return i;
}

class Student {
    private:
        char* name;
        int age;
    public:
        Student(const char* n = "Unknown", int age = 0);  // ✅ defaults HERE
        Student(const Student& other);
        ~Student();
        Student& operator=(const Student& other);

        void print();
};

Student :: Student(const char* n, int age)  {

    cout << "Constructor called for: " << n << endl;
    name = new char[mystrlen(n) + 1];
    mystrcpy(n, name);
    this-> age = age;
}

Student::Student(const Student& other) {
    cout << "Copy constructor called for: " << other.name << endl;
    name = new char[mystrlen(other.name) + 1];
    mystrcpy(other.name, name);
    age = other.age;
}

Student :: ~Student()   {
    delete[] name;
}

Student& Student :: operator=(const Student& other) {

    cout << "operator= called" << endl;
    if (this == &other) return *this;

    delete[] name;
    name = new char[mystrlen(other.name) + 1];
    mystrcpy(other.name, name);
    age = other.age;

    return *this;
}

void Student :: print()    {
    cout << name << ", " << age << endl;
}

int main()  {
    Student s1("Asad" , 18);
    Student s2;

    s1 = s2;

    s1.print();
    s2.print();

    system("pause");
    return 0;
}