# include <iostream>
using namespace std;

int ln(const char*);
char* cpy(const char*);

class BankAccount {
    private:
        char* ownerName;
        double balance;
        static int totalAccounts;
    public:
        BankAccount();
        BankAccount(char*, double);
        ~BankAccount();

        // methods
        static int getTotalAccounts();
        void deposit(double);
        friend ostream& operator<<(ostream&, const BankAccount&);
};

int BankAccount :: totalAccounts = 0;

BankAccount :: BankAccount()    {
    ownerName = cpy("Unknown");
    balance = 0;
    totalAccounts++;
}

BankAccount :: BankAccount(char* name, double amt)  {
    ownerName = cpy(name);
    balance = amt;
    totalAccounts++;
}

BankAccount :: ~BankAccount()   {
    delete[] ownerName;
    totalAccounts--;
}

int BankAccount :: getTotalAccounts()   {
    return totalAccounts;
} 

void BankAccount :: deposit(double bal)    {
    if (bal <= 0)    {
        cout << "Invalid Amount Added! Amount should be greater than 0!" << endl;
    }
    balance += bal;
}

ostream& operator<<(ostream& out, const BankAccount& dummy) {
    out << "Account: " << dummy.ownerName << " | Balance: " << dummy.balance << endl;
    return out;
}

int main()  {
    BankAccount acc1("Ali",1000);
    BankAccount acc2("Sara",800);

    cout << "Total Accounts: " << BankAccount :: getTotalAccounts() << endl;

    acc1.deposit(500);

    cout << acc1;
    cout << acc2;

    return 0;
}

int ln(const char* str) {
    int i = 0;
    while (str[i] != '\0')  i++;
    return i;
}

char* cpy(const char* src)  {
    char* dest = new char [ln(src) + 1];
    int i = 0;
    while (src[i] != '\0')  {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}