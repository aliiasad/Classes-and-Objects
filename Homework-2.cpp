# include <iostream>
# include <conio.h>
# include <cmath>
using namespace std;

class ComplexNumber    {
    private:
        int* real;
        int* imaginary;
    public:
        ComplexNumber(int, int);   // constructor (overloaded)
        ComplexNumber(const ComplexNumber&); // copy constructor
        ~ComplexNumber();  // destructor

        // functions to implement
        void input();
        void output();
        ComplexNumber& operator= (const ComplexNumber&);
        bool isEqual(const ComplexNumber&);
        ComplexNumber Conjugate ();
        ComplexNumber Add(const ComplexNumber&);
        ComplexNumber Subtract(const ComplexNumber&);
        ComplexNumber Multiplication(const ComplexNumber&);
        double magnitude();

};

ComplexNumber :: ComplexNumber(int r = 0, int i = 0)  {
    real = new int(r);
    imaginary = new int(i);
}

ComplexNumber :: ComplexNumber(const ComplexNumber& dummy)   {
    real = new int(*(dummy.real));
    imaginary = new int(*(dummy.imaginary));
}

ComplexNumber& ComplexNumber :: operator= (const ComplexNumber& dummy)    {
    if (this == &dummy) return *this;

        *real = *(dummy.real);
        *imaginary = *(dummy.imaginary);

    return *this;
}

ComplexNumber :: ~ComplexNumber ()    {
    delete real;
    delete imaginary;
}

bool ComplexNumber :: isEqual(const ComplexNumber& dummy)  {
    return (*real == *(dummy.real) && *imaginary == *(dummy.imaginary));
}

ComplexNumber ComplexNumber :: Conjugate()   {
    return ComplexNumber(*real,-*imaginary);
}

ComplexNumber ComplexNumber :: Add(const ComplexNumber& dummy) {
    return ComplexNumber((*real + *(dummy.real)), (*imaginary + *(dummy.imaginary)));
}

ComplexNumber ComplexNumber :: Subtract(const ComplexNumber& dummy)    {
     return ComplexNumber((*real - *(dummy.real)), (*imaginary - *(dummy.imaginary)));
}

ComplexNumber ComplexNumber :: Multiplication(const ComplexNumber& dummy)  {
    int realPart = (*real * *(dummy.real)) - (*imaginary * *(dummy.imaginary));
    int imgPart = (*real * *(dummy.imaginary)) + (*imaginary * *(dummy.real));

    return ComplexNumber(realPart, imgPart);
}

double ComplexNumber :: magnitude()   {
    return sqrt((*real * *real) + (*imaginary * *imaginary));
}


void ComplexNumber :: input() {
    int r, i;

    cout << "Enter Real: ";
    cin >> r;
    cout << "Enter Imaginary: ";
    cin >> i;

    *real = r;
    *imaginary = i;
}

void ComplexNumber :: output ()    {
    if (*imaginary >= 0)  {
        cout << *real << " + " << *imaginary << "i" << endl;
    }
    else cout << *real << " - " << -(*imaginary) << "i" << endl;
}


 int main() {
    ComplexNumber c1,c2;

    cout << "Enter c1: " << endl;
    c1.input();

    cout << "Enter c2: " << endl;
    c2.input();

    //printing
    cout << "c1 = ";
    c1.output();

    cout << "c2 = ";
    c2.output();
    

    // == or !=
    if (c1.isEqual(c2)) cout << "c1 is EQUAL to c2" << endl;
    else cout << "c1 is NOT EQUAL to c2" << endl;

    // conjugate
    cout << "Conjugate of c1: \t";
    c1.Conjugate().output();
    cout << "\nConjugate of c2: \t";
    c2.Conjugate().output();

    // c1 + c2
    cout << "c1 + c2: \t";
    c1.Add(c2).output();

    // c1 - c2
    cout << "c1 - c2: \t";
    c1.Subtract(c2).output();

    // c1 * c2
    cout << "c1 X c2: \t";
    c1.Multiplication(c2).output();

    // magnitude
    cout << "\nMagnitude of c1 =  " << c1.magnitude();
    cout << "\nMagnitude of c2 = " << c2.magnitude();
    

    cout << "Press any key to continue . . ." << endl;
    getch();
    return 0;
 }