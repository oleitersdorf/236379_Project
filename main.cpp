
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

typedef std::vector<int> vi;
using namespace std;

/* --- Util. --- */

/***
 * Converts given integer to binary vector
 * @param i the integer
 * @param N the representation size
 * @return the binary vector
 */
vi toBinary(int i, int N){
    vi out (N);
    for (int j = 0; j < N; j++){
        out[j] = (i & (1 << j)) >> j;
    }
    return out;
}

/**
 * Converts the given number from binary vector to integer
 * @param bin the binary vector
 * @param N the representation size
 * @return the integer value
 */
int fromBinary(vi bin, int N){
    int out = 0;
    for (int j = 0; j < N; j++){
        out += (bin[j] << j);
    }
    return out;
}

/* --- Period Computation --- */

/**
 * Performs the Z algorithm on the given input
 * @param s the input
 * @return the Z vector
 */
vi ComputeZ(const vi& s){
    size_t n = s.size();
    vi z(n);
    int L = 0, R = 0;
    for (int i = 1; i < n; i++) {
        if (i > R) {
            L = R = i;
            while (R < n && s[R-L] == s[R]) R++;
            z[i-1] = R-L; R--;
        } else {
            int k = i-L;
            if (z[k-1] < R-i+1) z[i-1] = z[k-1];
            else {
                L = i;
                while (R < n && s[R-L] == s[R]) R++;
                z[i-1] = R-L; R--;
            }
        }
    }
    return z;
}
/**
 * Returns all periods (continuous) satisfies by the input
 * @param s the input
 * @return the vector of periods
 */
vi ComputePeriods(const vi& s){
    size_t n = s.size();
    vi periods;
    vi z = ComputeZ(s);
    for(int i = 1; i <= n; i++){
        if (i + z[i-1] == n){
            periods.push_back(i);
        }
    }
    return periods;
}
/**
 * Returns the minimal period (continuous) satisfied by the input
 * @param s the input
 * @return the minimal period
 */
int ComputeMinPeriod(const vi& s){
    vi periods = ComputePeriods(s);
    return *std::min_element(periods.begin(), periods.end());
}

/* --- Proposed Encoder/Decoder --- */

/**
 * Encodes the given input with a single-redundancy bit,
 * such that no l-window contains a period < p
 * @param in the input
 * @param n the size of the input
 * @param l the desired window size
 * @param p the period bound
 * @return the encoded data
 */
vi encode(const vi& in, int n, int l, int p){

    int log2_n = ceil(log2(n));

    // Concatenate a 1 to the end
    vi out = in;
    out.insert(out.end(), 1);

    // Search for violations, add index to end
    while(true){

        bool found = false;

        for(int i = 0; i < (n+1)-(l-1); i++){

            vi v_sub = vi(out.begin() + i, out.begin() + i + l);

            int period = ComputeMinPeriod(v_sub);

            if(period < p){ // period in {1, 2, ..., p-1}

                // Erase the repeated parts
                out.erase(out.begin() + i + p, out.begin() + i + l);

                out[i + period] = 1;
                for (int j = i + period + 1; j < i + p; j++){
                    out[j] = 0;
                }

                // Insert the index i to the end
                vi index_encoding = toBinary(i, log2_n);
                out.insert(out.end(), index_encoding.begin(), index_encoding.end());

                // Insert 0 to the end
                out.insert(out.end(), 0);

                assert(out.size() == n + 1);

                found = true;

            }

        }

        if (!found) break;

    }

    return out;
}

/**
 * Decodes the given output to receive the original message.
 * @param out the encoded input
 * @param n the size of the input
 * @param l the desired window size
 * @param p the period bound
 * @return the original input
 */
vi decode(const vi& out, int n, int l, int p){

    int log2_n = ceil(log2(n));

    vi in = out;

    while(in[n] == 0){

        // Remove the zero
        in.erase(in.end() - 1);

        // Read and erase the index
        vi index_encoding = vi(in.end() - log2_n, in.end());
        int index = fromBinary(index_encoding, log2_n);
        in.erase(in.end() - log2_n, in.end());

        int j = index + p - 1;
        while(in[j] == 0) {
            j--;
        }
        int period = j - index;

        // Unwrap the period
        in.insert(in.begin() + index + p, l-p, 0);
        for (int i = index + period; i < index + l; i++){
            in[i] = in[i-period];
        }

        assert(in.size() == (n+1));

    }

    // Drop the 1 at the end
    in.erase(in.end() - 1);

    return in;
}

/* --- Testing --- */

int main() {

    // Parameters
    int n = 20;
    int p = 14; // No periods of size < p

    int min_l = (int)(ceil(log2(n))) + p + 1;
    int l = min_l;
    assert(l >= min_l);

    cout << "Parameters: n=" << n << ", l=" << l << ", p=" << p << " (where min_l = " << min_l << ")" << endl;

    // Brute-force search on all inputs
    for (int num = 0; num < (1 << n); num++) {

        // Construct the input
        vi v(n);
        for (int i = 0; i < n; i++) v[i] = (num & (1 << i)) >> i;

        // Encode
        vi encoded = encode(v, n, l, p);

        // Verify encoded satisfies period requirement
        for (int i = 0; i < (n+1)-(l-1); i++){
            vi v_sub = vi(encoded.begin() + i, encoded.begin() + i + l);
            assert(ComputeMinPeriod(v_sub) >= p);
        }

        // Decode
        vi decoded = decode(encoded, n, l, p);
        assert(decoded == v);

        if (num % ((1 << n) / 100) == 0){

            int progress = num / ((1 << n) / 100);

            std::cout << "[";
            for (int i = 0; i < 100; ++i) {
                if (i < progress) std::cout << "=";
                else if (i == progress) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << progress << " %\r";
            std::cout.flush();
        }

    }

}
