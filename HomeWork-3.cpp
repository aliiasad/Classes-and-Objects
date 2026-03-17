# include <iostream>
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

bool MyString::operator<(MyString dummy) {
    int i = 0;
    while (str[i] != '\0' && dummy.str[i] != '\0') {
        if (str[i] < dummy.str[i]) return true;
        if (str[i] > dummy.str[i]) return false;
        i++;
    }
    return length < dummy.length;
}

int main()  {

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