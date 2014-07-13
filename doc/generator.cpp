/*
 * sample.
 *
 * Something more or less like Python generators.
 */
#include <cstdlib>
#include <cstdio>
template<class base> class fi /* filler_iterator */ {
        private:
        base &parent;
        public:
        fi(base &p) : parent(p) {};
        bool operator!=(const fi &) { return parent.has_more(); };
        const fi &operator++(void) { return *this; };
        int operator*(void) { return parent.yield(); };
};

class rdgen {
        private:
        unsigned int remaining;

        public:
        rdgen(unsigned int x) {
                remaining = x;
        };
        bool has_more(void) {
                return remaining > 0;
        };
        int yield(void) {
                --remaining;
                return rand();
        };

        fi<rdgen> begin(void) { return fi<rdgen>(*this); };
        fi<rdgen> end(void) { return fi<rdgen>(*this); };
};

int main(int argc, const char **argv)
{
        for (auto x : rdgen(argc))
                printf("%d\n", x);
        return 0;
}
