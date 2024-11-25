
/** 
 *   Demo FLINT program to demonstrate some use of the padic module.
 */


#include <gmp.h>
#include <flint/flint.h>
#include <flint/fmpz.h>
#include <flint/fmpz_mod_poly.h>
#include <flint/aprcl.h>
#include <flint/padic.h>


#include <memory>
#include <string>
#include <stdexcept>

#include "exprtk.hpp"
#include "acutest.h"
#include <iostream>
#include <limits>

// FLINT C++ wrapper

namespace flint 
{
    using unsigned_long_t = mp_limb_t;       // width of one segment of an unsigned GMP multi-precision integer
    using signed_long_t = mp_limb_signed_t;  // width of one segment of a signed GMP multi-precision integer

    enum class PadicPrintMode : uint8_t
    {
        TERSE = PADIC_TERSE,
        SERIES = PADIC_SERIES,
        VAL_UNIT = PADIC_VAL_UNIT
    };


    class Base 
    {
    private:
        int _b;
    public:
        explicit Base(int b) : _b(b) 
        {
            if(b < 2 || b > 62)
            {
                throw std::invalid_argument("Base must be between 2 and 62.");
            }
        }

        // cast to int
        explicit operator int() const 
        {
            return _b;
        }

    };

    //! @brief Wrapper class for the FLINT fmpz_t type.
    class Fmpz 
    {
    private:
        fmpz_t _val;

    public:
        //! @brief Default constructor.
        //! @details Initializes the fmpz_t value to zero.
        Fmpz() 
        {
            fmpz_init(_val);
        }

        //! @brief Constructor with pre-allocated limbs (memory).
        //! @param limbs The number of limbs to allocate.
        explicit Fmpz(const unsigned_long_t limbs) 
        {
            fmpz_init2(_val, limbs);
        }

        //! @brief Set the value of the fmpz_t to an unsigned long.
        //! @param val The value to set the fmpz_t to.
        void set(const unsigned_long_t val) 
        {
            fmpz_set_ui(_val, val);
        }

        //! @brief Set the value of the fmpz_t to a signed long.
        //! @param val The value to set the fmpz_t to.
        void set(const signed_long_t val) 
        {
            fmpz_set_si(_val, val);
        }

        const fmpz_t& get() const
        {
            return _val;
        }

        //! @brief Print the value of the fmpz_t to a string.
        //! @param b The base to print the value in.
        std::string toString(const Base b) const
        {
            char* str = fmpz_get_str(nullptr, static_cast<int>(b), _val);
            return std::string(str);
        }

        //! @brief Check if the value of the fmpz_t is prime.
        //! @return True if the value is prime, false otherwise.
        bool isPrime() const 
        {
            return aprcl_is_prime(_val);
        }

        ~Fmpz() 
        {
            fmpz_clear(_val);
        }

        friend std::ostream& operator<<(std::ostream& os, const Fmpz& x)
        {
            os << x.toString(Base(10));
            return os;
        }
    };

    class PadicContext 
    {
    private:
        padic_ctx_t _ctx;

    public:

        //! @param p The prime number.
        //! @param min The minimum number of pre-computed powers of p to store.
        //! @param max The maximum number of pre-computed powers of p to store.
        explicit PadicContext(const Fmpz& p, signed_long_t min = 8, signed_long_t max = 12) 
        {
            if(!p.isPrime())
            {
                throw std::invalid_argument("The prime number must be a prime number.");
            }
            padic_ctx_init(_ctx, p.get(), min, max, PADIC_TERSE);
        }

        ~PadicContext() 
        {
            padic_ctx_clear(_ctx);
        }

        //! @brief Set the print mode for the PadicNumber.
        //! @param mode The print mode to set.
        void setPrintMode(PadicPrintMode mode) 
        {
           _ctx->mode = static_cast<padic_print_mode>(mode);
        }

        const padic_ctx_t& get() const
        {
            return _ctx;
        }
    };

    class PadicNumber 
    {
    private:
        std::shared_ptr<PadicContext> _ctx;
        padic_t _val;

        const padic_ctx_t& _getContext() const
        {
            return _ctx.get()->get();
        }

        std::shared_ptr<PadicContext> getContext() const
        {
            return _ctx;
        }

    public:

        //! @brief Constructor.
        //! @param prec The precision to use (default is PADIC_DEFAULT_PREC = 20).
        explicit PadicNumber(std::shared_ptr<PadicContext> ctx, signed_long_t prec = PADIC_DEFAULT_PREC) : _ctx(ctx)
        {
            padic_init2(_val, prec);
        }

        ~PadicNumber() 
        {
            padic_clear(_val);
        }

        //! @brief Set the value of the padic_t to an unsigned long.
        //! @param val The value to set the padic_t to.
        void set(const unsigned_long_t val) 
        {
            padic_set_ui(_val, val, _getContext());
        }

        //! @brief Set the value of the padic_t to a signed long.
        //! @param val The value to set the padic_t to.
        void set(const signed_long_t val) 
        {
            padic_set_si(_val, val, _getContext());
        }
        
        //! @brief Print the value of the fmpz_t to a string.
        //! @param b The base to print the value in.
        std::string toString(const PadicPrintMode& mode) 
        {
            _ctx->setPrintMode(mode);
            char* str = padic_get_str(nullptr, _val, _getContext());
            return std::string(str);
        }


        friend PadicNumber operator + (const PadicNumber& lhs, const PadicNumber& rhs); 

        friend PadicNumber log(const PadicNumber& x, signed_long_t prec);
        friend PadicNumber exp(const PadicNumber& x, signed_long_t prec);

        friend std::ostream& operator<<(std::ostream& os, PadicNumber& x)
        {
            os << x.toString(PadicPrintMode::TERSE);
            return os;
        }
    };

    PadicNumber operator + (const PadicNumber& lhs, const PadicNumber& rhs) 
    {
        PadicNumber y(lhs.getContext());
        padic_add(y._val, lhs._val, rhs._val, lhs._getContext());
        return y;
    }

    PadicNumber log(const PadicNumber& x, signed_long_t prec = PADIC_DEFAULT_PREC) 
    {
        PadicNumber y(x.getContext(), prec);
        auto res = padic_log(y._val, x._val, x._getContext());
        if(res != 1)
        {
            throw std::runtime_error("Error computing the log.");
        }
        return y;
    }

    PadicNumber exp(const PadicNumber& x, signed_long_t prec = PADIC_DEFAULT_PREC) 
    {
        PadicNumber y(x.getContext(), prec);
        auto res = padic_exp(y._val, x._val, x._getContext());
        if(res != 1)
        {
            throw std::runtime_error("Error computing the exp.");
        }
        return y;
    }
}

void test_case_1() 
{
#define P 7ull
#define X 127ull
#define PREC 10ull

    flint::Fmpz p;
    p.set( static_cast<flint::unsigned_long_t>(P) );

    auto ctx = std::make_shared<flint::PadicContext>(p);

    flint::PadicNumber padic(ctx, flint::signed_long_t(PREC));
    padic.set( static_cast<flint::unsigned_long_t>(X) );

    const std::string x_str_terse = padic.toString(flint::PadicPrintMode::TERSE);
    const std::string x_str_series = padic.toString(flint::PadicPrintMode::SERIES);

    TEST_CHECK(x_str_terse == "127");
    TEST_CHECK(x_str_series == "1 + 4*7^1 + 2*7^2");

    exprtk::expression<double> expr;
    exprtk::parser<double> parser;

    parser.compile(x_str_terse, expr);
    auto lhs = expr.value();

    parser.compile(x_str_series, expr);
    auto rhs = expr.value();
    
    TEST_CHECK(lhs == rhs);


    std::cout << "x ≡ 127 mod 7^10" << "\n";
    std::cout << "p = " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << "\n";
    std::cout << "x = " << x_str_terse << " (" << x_str_series << ")" << "\n";
    std::cout << "\n";
}

void test_case_2() 
{
#define P 2ull
#define X 1057ull
#define PREC 10ull

    flint::Fmpz p;
    p.set(static_cast<flint::unsigned_long_t>(P));

    auto ctx = std::make_shared<flint::PadicContext>(p);

    flint::PadicNumber padic(ctx, flint::signed_long_t(PREC));
    padic.set(static_cast<flint::unsigned_long_t>(X));

    const std::string x_str_terse = padic.toString(flint::PadicPrintMode::TERSE);
    const std::string x_str_series = padic.toString(flint::PadicPrintMode::SERIES);

    exprtk::expression<double> expr;
    exprtk::parser<double> parser;

    parser.compile(x_str_terse, expr);
    auto lhs = expr.value();

    parser.compile(x_str_series, expr);
    auto rhs = expr.value();
    
    TEST_CHECK(lhs == rhs);

    std::cout << "x ≡ 1057 mod 2^10" << "\n";
    std::cout << "p = " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << "\n";
    std::cout << "x = " << x_str_terse << " (" << x_str_series << ")" << "\n";
    std::cout << "\n";
}

void test_case_3() 
{
#define P 3ull
#define X -127ll
#define PREC 10ull

    flint::Fmpz p;
    p.set(static_cast<flint::unsigned_long_t>(P));

    auto ctx = std::make_shared<flint::PadicContext>(p);

    flint::PadicNumber padic(ctx, flint::signed_long_t(PREC));
    padic.set(static_cast<flint::signed_long_t>(X));

    const std::string x_str_terse = padic.toString(flint::PadicPrintMode::TERSE);
    const std::string x_str_series = padic.toString(flint::PadicPrintMode::SERIES);

    TEST_CHECK(x_str_terse == "58922");    
    TEST_CHECK(x_str_series == "2 + 2*3^1 + 1*3^3 + 1*3^4 + 2*3^5 + 2*3^6 + 2*3^7 + 2*3^8 + 2*3^9");

    exprtk::expression<double> expr;
    exprtk::parser<double> parser;

    parser.compile(x_str_terse, expr);
    auto lhs = expr.value();

    parser.compile(x_str_series, expr);
    auto rhs = expr.value();
    
    TEST_CHECK(lhs == rhs);

    std::cout << "x ≡ -127 mod 3^10" << "\n";
    std::cout << "p = " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << "\n";
    std::cout << "x = " << x_str_terse << " (" << x_str_series << ")" << "\n";
    std::cout << "\n";
}


void test_logarithm() 
{
#define P 5ull
#define X 7380996ull

    flint::Fmpz p;
    p.set(static_cast<flint::unsigned_long_t>(P));

    auto ctx = std::make_shared<flint::PadicContext>(p, 10, 25);

    flint::PadicNumber x(ctx);
    x.set(static_cast<flint::unsigned_long_t>(X));

    auto y = flint::log(x);

    const std::string x_str_terse = x.toString(flint::PadicPrintMode::TERSE);
    const std::string x_str_series = x.toString(flint::PadicPrintMode::SERIES);

    const std::string y_str_terse = y.toString(flint::PadicPrintMode::TERSE);
    const std::string y_str_series = y.toString(flint::PadicPrintMode::SERIES);

    TEST_CHECK(x_str_series == "1 + 4*5^1 + 4*5^2 + 2*5^3 + 4*5^4 + 1*5^5 + 2*5^6 + 4*5^7 + 3*5^8 + 3*5^9");
    TEST_CHECK(y_str_series == "4*5^1 + 1*5^2 + 3*5^3 + 1*5^4 + 1*5^5 + 4*5^6 + 1*5^7 + 1*5^8 + 2*5^9 + 1*5^10 + 3*5^11 + 4*5^12 + 4*5^13 + 1*5^14 + 2*5^15 + 4*5^18 + 2*5^19");

    exprtk::expression<double> expr;
    exprtk::parser<double> parser;

    parser.compile(y_str_terse, expr);
    auto lhs = expr.value();

    parser.compile(y_str_series, expr);
    auto rhs = expr.value();
    
    TEST_CHECK(lhs == rhs);


    std::cout << "log(7380996) mod 5^20" << "\n";
    std::cout << "p = " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << "\n";
    std::cout << "x =" << x_str_terse << " (" << x_str_series << ")" << "\n";
    std::cout << "log(x) = " << y_str_terse << " (" << y_str_series << ")" << "\n";
    std::cout << "\n";
}


void test_exp() 
{

    flint::Fmpz p;
    p.set(static_cast<flint::unsigned_long_t>(2));

    auto ctx = std::make_shared<flint::PadicContext>(p, 10, 25);

    flint::PadicNumber x(ctx);
    x.set(static_cast<flint::unsigned_long_t>(4));

    auto y = flint::exp(x);

    const std::string x_str_terse = x.toString(flint::PadicPrintMode::TERSE);
    const std::string x_str_series = x.toString(flint::PadicPrintMode::SERIES);

    const std::string y_str_terse = y.toString(flint::PadicPrintMode::TERSE);
    const std::string y_str_series = y.toString(flint::PadicPrintMode::SERIES);

    TEST_CHECK(x_str_terse == "4");
    TEST_CHECK(y_str_terse == "934221");
    TEST_CHECK(x_str_series == "1*2^2");
    TEST_CHECK(y_str_series == "1 + 1*2^2 + 1*2^3 + 1*2^6 + 1*2^8 + 1*2^14 + 1*2^17 + 1*2^18 + 1*2^19");


    exprtk::expression<double> expr;
    exprtk::parser<double> parser;

    parser.compile(y_str_terse, expr);
    auto lhs = expr.value();

    parser.compile(y_str_series, expr);
    auto rhs = expr.value();
    
    TEST_CHECK(lhs == rhs);


    std::cout << "exp(4) mod 2^10" << "\n";
    std::cout << "p: " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << "\n";
    std::cout << "x = " << x_str_terse << " (" << x_str_series << ")" << "\n";
    std::cout << "exp(x) = " << y_str_terse << " (" << y_str_series << ")" << "\n";
    std::cout << "\n";
}

bool multiply_with_overflow_check(flint::unsigned_long_t a, flint::unsigned_long_t b, flint::unsigned_long_t& result) {
    if (a > 0 && b > std::numeric_limits<flint::unsigned_long_t>::max() / a) {
        return false; // Overflow would occur
    }
    result = a * b;
    return true;
}

void test_add() 
{
    std::cout << "x + y" << "\n";

    flint::unsigned_long_t x_int = 1ull;
    for (flint::unsigned_long_t p = 2ull; p <= 47ull; p++)
    {

        flint::Fmpz prime;
        prime.set(p);
        if (prime.isPrime())
        {
            if (!multiply_with_overflow_check(x_int, p, x_int)) 
            {
                throw std::overflow_error("Integer overflow detected");
            }
            std::cout << p << ": " << x_int << "\n";
        }
    }

    for (flint::unsigned_long_t p = 2ull; p <= 47ull; p++)
    {

        flint::Fmpz prime;
        prime.set(p);
        if (prime.isPrime())
        {
            std::cout << "p: " << prime << " (0b" << prime.toString(flint::Base(2)) << ")" << "\n";

            auto ctx = std::make_shared<flint::PadicContext>(prime, 0, 1);

            flint::PadicNumber x(ctx);
            x.set(static_cast<flint::unsigned_long_t>(x_int));
            std::cout << "x = " << x << " (" << x.toString(flint::PadicPrintMode::SERIES) << ")" << "\n";

            flint::PadicNumber y(ctx);
            y.set(static_cast<flint::unsigned_long_t>(1));
            std::cout << "y = " << y << " (" << y.toString(flint::PadicPrintMode::SERIES) << ")" << "\n";

            auto z = x + y;
            std::cout << "x + y = " << z << " (" << z.toString(flint::PadicPrintMode::SERIES) << ")" << "\n";
            std::cout << "\n";
        }
    }
}


TEST_LIST = {
   { "test_case_1", test_case_1 },
   { "test_case_2", test_case_2 },
   { "test_case_2", test_case_3 },
   { "test_logarithm", test_logarithm },
   { "test_exp", test_exp },
   { "test_add", test_add },

   { NULL, NULL }     /* zeroed record marking the end of the list */
};