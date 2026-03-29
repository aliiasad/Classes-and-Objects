    # include <iostream>
    using namespace std;

    class flexibleVectors{
        private:
            double* components;
            int dimensions; // size
        public:
            flexibleVectors(); // def
            flexibleVectors(int);  // para
            flexibleVectors(const flexibleVectors&);    // cpy
            ~flexibleVectors(); // des

            // req
            friend istream& operator>>(istream& in, flexibleVectors&);
            friend ostream& operator<<(ostream& out, const flexibleVectors&);
            flexibleVectors& operator=(const flexibleVectors&);
            flexibleVectors operator+(const flexibleVectors&);
            friend flexibleVectors operator*(int, const flexibleVectors&);
    };

    flexibleVectors :: flexibleVectors()   {
        dimensions = 0;

        components = new double [dimensions];
        for (int i = 0; i < dimensions; i++)  {
            *(components + i) = 0;
        }
    }

    flexibleVectors :: flexibleVectors(int size)   {
        dimensions = size;

        components = new double [dimensions];
        for (int i = 0; i < dimensions; i++)  {
            *(components + i) = 0;
        }
    }

    flexibleVectors :: flexibleVectors(const flexibleVectors& dummy) {
        dimensions = dummy.dimensions;

        components = new double [dimensions];
        for (int i = 0; i < dimensions; i++)  {
            *(components + i) = *(dummy.components + i);
        }
    }

    flexibleVectors& flexibleVectors:: operator=(const flexibleVectors& dummy)  {
        if (this == &dummy) return *this;

        delete[] components;

        dimensions = dummy.dimensions;

        components = new double [dimensions];
        for (int i = 0; i < dimensions; i++)    {
            *(components + i) = *(dummy.components + i);
        }

        return *this;
    }

    flexibleVectors flexibleVectors :: operator+(const flexibleVectors& dummy)   {
        if (dimensions > dummy.dimensions)  {
            int maxDim = dimensions;
            double* newVector = new double [dimensions];
            for (int i = 0; i < dimensions; i++)    {
                if (i < dimensions - dummy.dimensions)  {
                    *(newVector + i) = *(components + i);
                }
                else    {
                    *(newVector + i) = *(dummy.components + (i - (dimensions - dummy.dimensions))) + *(components + i);
                }
            }
            flexibleVectors result(maxDim);
            for (int i = 0; i < maxDim; i++)
                *(result.components + i) = *(newVector + i);
            delete[] newVector;
            return result;
        }
        else if (dimensions < dummy.dimensions) {
            int maxDim = dummy.dimensions;
            double* newVector = new double [dummy.dimensions];
            for (int i = 0; i < dummy.dimensions; i++)    {
                if (i < dummy.dimensions - dimensions)  {
                    *(newVector + i) = 0 + *(dummy.components + i);
                }
                else    {
                    *(newVector + i) = *(dummy.components + i) + *(components + (i - (dummy.dimensions - dimensions)));
                }
            }
            flexibleVectors result(maxDim);
            for (int i = 0; i < maxDim; i++)
                *(result.components + i) = *(newVector + i);
            delete[] newVector;
            return result;
        }
        else {
            int maxDim = dimensions;
            double* newVector = new double[dimensions];
            for (int i = 0; i < dimensions; i++)
                *(newVector + i) = *(components + i) + *(dummy.components + i);
            flexibleVectors result(maxDim);
            for (int i = 0; i < maxDim; i++)
                *(result.components + i) = *(newVector + i);
            delete[] newVector;
            return result;
        }
    }

    istream& operator>>(istream& in, flexibleVectors& dummy)  {
        for (int i = 0; i < dummy.dimensions; i++)  {
            in >> dummy.components[i];
        }
        return in;
    }

    ostream& operator<<(ostream& out, const flexibleVectors& dummy)   {
        out << "[ ";
        for (int i = 0; i < dummy.dimensions; i++) {
            if (i == dummy.dimensions - 1)   {
                out << dummy.components[i] << "]";
            }
            else {
                out << dummy.components[i] << ", ";
            }
        }
        return out;
    }

    flexibleVectors operator*(int num, const flexibleVectors& dummy)  {
        double* newVector = new double [dummy.dimensions];
        for (int i = 0; i < dummy.dimensions; i++)  {
            newVector[i] = num * dummy.components[i];
        }
        flexibleVectors result(dummy.dimensions);
        for (int i = 0; i < dummy.dimensions; i++)  {
            result.components[i] = newVector[i];
        }
        delete[] newVector;
        return result;
    }

    flexibleVectors :: ~flexibleVectors()   {
        delete[] components;
    }

    int main()  {
        flexibleVectors v1, v2(2), v3(5), v4;
        cin >> v2 >> v3;
        v4 = v3;
        v1 = v2 + v3;
        cout << v1 << endl;
        cout << 3 * v4 << endl; 
        return 0;
    }