# include <iostream>
# include <string>
using namespace std;

int len(const char* str);
void cpy(const char* src, char* dest);
char* dynamic_cpy(const char* src);

class MyString {
    private:
        char* str;
        int length;

        // static helpers
        static char* GetStringFromBuffer()  {
            char sentence[5000];

            char temp;
            int i = 0;
            while (cin.get(temp) && temp != '\n')  {
                sentence[i++] = temp; 
            }
            sentence[i] = '\0';

            return dynamic_cpy(sentence);
        }

        static char* Concatenate(const char* str1, const char* str2)  {
            char* result = new char [len(str1) + len(str2) + 1];

            int i = 0;
            while (str1[i] != '\0')     {
                result[i] = str1[i];
                i++;
            }
            int j = 0;
            while (str2[j] != '\0')    {
                result[i] = str2[j];
                i++;
                j++;
            }
            result[i] = '\0';

            return result;
        }
    public:
        MyString();
        MyString(const char*);
        MyString(const MyString&);
        MyString(MyString&&); // move constructor
        ~MyString();

        // predefined
        MyString operator+(const MyString);
        MyString& operator=(const MyString&);
        bool operator<(MyString);

        // required
        bool operator!();
        friend istream& operator>>(istream& in, MyString&);
        friend ostream& operator<<(ostream& out, const MyString&);
        char& operator[](int);
        MyString operator()(int, int);
};

MyString :: MyString()  {
    str = dynamic_cpy("");
    length = 0;
}

MyString::MyString(const char* s) {
    length = len(s);
    str = dynamic_cpy(s);
}

MyString :: MyString(const MyString& dummy) {
    str = dynamic_cpy(dummy.str);
    length = dummy.length;
}

MyString :: MyString(MyString&& dummy)  {
    str = dummy.str;
    length = dummy.length;
    dummy.str = nullptr;
    dummy.length = 0;
}

MyString :: ~MyString() {
    delete[] str;
}

MyString& MyString :: operator=(const MyString& dummy)  {
    if (this == &dummy) return *this;

    delete[] str;
    str = dynamic_cpy(dummy.str);
    length = dummy.length;

    return *this;
}

MyString MyString :: operator+(const MyString dummy)    {
    char* result = Concatenate(str, dummy.str);
    MyString temp(result);
    delete[] result;
    return temp;
}

bool MyString :: operator!()    {
    return length == 0; // if length is zero -> ! returns true
}

istream& operator>>(istream& in, MyString& dummy)   {
    delete[] dummy.str;

    dummy.str = MyString :: GetStringFromBuffer();
    dummy.length = len(dummy.str);

    return in;
}

ostream& operator<<(ostream& out, const MyString& dummy) {
    out << dummy.str;

    return out;
}

MyString MyString :: operator()(int startIdx, int subLength)    {
    // check availability
    int available = length - startIdx;
    if (subLength < 0)  subLength = 0;
    if (subLength > available)  subLength = available;

    char* result = new char [subLength + 1];
    int i = 0;
    while (i < subLength)   {
        result[i] = str[startIdx + i];
        i++;
    }
    result[i] = '\0';

    MyString temp(result);
    delete[] result;
    return temp;
}

char& MyString :: operator[](int idx)   {
    return str[idx];
}

bool MyString :: operator<(MyString dummy) {
    int i = 0;
    while (str[i] != '\0' && dummy.str[i] != '\0') {
        if (str[i] < dummy.str[i]) return true;
        if (str[i] > dummy.str[i]) return false;
        i++;
    }
    return length < dummy.length;
}

int main()  {
    MyString str1, str2, str3, str4; //Default constructor will make a string of 
    //lenght 0 but having null character (only) i.e. empty string
    if(!str1)   {
    cout<<"String 1 is Empty.\n";
    cout<<"Str 1 = "<<str1<<endl<<endl<<endl;
    }

    cout << "Enter String 1:\t";
    cin >> str1;
    cout << "Enter String 2:\t";
    cin >> str2;
    cout << "\n\n\nUser Entered:\n";
    cout << "String 1 = " << str1 << endl;
    cout << "String 2 = " << str2 << endl<<endl<<endl;
    //What is following code testing?
    cout<<"Before str1 = str1; str1 = "<<str1<<endl;
    str1 = str1;
    cout<<"After str1 = str1, str1 = "<<str1<<endl<<endl<<endl;
    cout<<"Before str4 = str3 = str1+str2\n";
    cout<<"str1 = "<<str1<<endl;
    cout<<"str2 = "<<str2<<endl;
    cout<<"str3 = "<<str3<<endl;
    cout<<"str4 = "<<str4<<endl;
    str4 = str3 = str1+str2;
    cout<<"\n\n\nAfter str4 = str3 = str1+str2\n";
    cout<<"str1 = "<<str1<<endl;
    cout<<"str2 = "<<str2<<endl;
    cout<<"str3 = "<<str3<<endl;
    cout<<"str4 = "<<str4<<endl;
    cout<<"\n\n\nEnter String 3:\t";
    cin >> str3;
    cout<<"\n\n\nEnter String 4:\t";
    cin >> str4;
    cout<<"\n\n\nstr3 = "<<str3<<endl;
    cout<<"str4 = "<<str4<<endl;
    if(str3 < str4)
    cout<<"String 3 is Less than String 4.\n";
    else
    cout<<"String 3 is NOT Less than String 4.\n";
    MyString str5 = str1 + str2;
    cout << "\n\n\nStr5:\t" << str5 << endl;
    cout << "Str5[7]:\t" << str5[7] << endl; //Function Call: str5.operator[](7).
    str5[7] = '$';
    cout << "\n\nStr5:\t" << str5 << endl;
    cout << "\n\n\nstr5(5, 10):\t" << str5(5, 10) << endl;// Substring of lenght 10 
    //starting from index 5 . Function Call str5.operator()(5,10) Let the returned MyString or 
    //char* leak
    return 0;
}

int len(const char* str)    {
    int i = 0;
    while (str[i] != 0) i++;
    return i;
}

char* dynamic_cpy(const char* src)  {
    char* dest = new char [len(src) + 1]; // +1 for \0

    int i = 0;
    while (src[i] != '\0')    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

void cpy(const char* src, char* dest)   {
    int i = 0;
    while (src[i] != '\0')  {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return;
}