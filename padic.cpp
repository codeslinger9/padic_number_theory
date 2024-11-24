
/*
    Demo FLINT program to demonstrate some use of the padic module.
*/

#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>

#include <flint/flint.h>
#include <flint/fmpz.h>
#include <flint/fmpz_mod_poly.h>
#include <flint/aprcl.h>
#include <flint/padic.h>

#include <iostream>
#include <memory>
#include <string>


namespace flint 
{
    using unsigned_long_t = mp_limb_t;
    using signed_long_t = mp_limb_signed_t;

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
        Base(int b) : _b(b) 
        {
            if(b < 2 || b > 62)
            {
                throw std::invalid_argument("Base must be between 2 and 62.");
            }
        }

        // cast to int
        operator int() const 
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
        Fmpz(const unsigned_long_t limbs) 
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

    class PadicNumber 
    {
    private:
        padic_ctx_t _ctx;
        padic_t _val;

        //! @brief Set the print mode for the PadicNumber.
        //! @param mode The print mode to set.
        void setPrintMode(PadicPrintMode mode) 
        {
           _ctx->mode = static_cast<padic_print_mode>(mode);
        }

    public:
        //! @brief Constructor.
        //! @param p The prime number.
        //! @param prec The precision to use (default is PADIC_DEFAULT_PREC = 20).
        //! @param min The minimum number of pre-computed powers of p to store.
        //! @param max The maximum number of pre-computed powers of p to store.
        PadicNumber(const Fmpz& p, signed_long_t prec = PADIC_DEFAULT_PREC, signed_long_t min = 8, signed_long_t max = 12)
        {
            if(!p.isPrime())
            {
                throw std::invalid_argument("The prime number must be a prime number.");
            }

            padic_init2(_val, prec);
        }

        //! @brief Set the value of the padic_t to an unsigned long.
        //! @param val The value to set the padic_t to.
        void set(const unsigned_long_t val) 
        {
            padic_set_ui(_val, val, _ctx);
        }

        //! @brief Set the value of the padic_t to a signed long.
        //! @param val The value to set the padic_t to.
        void set(const signed_long_t val) 
        {
            padic_set_si(_val, val, _ctx);
        }
        
        //! @brief Print the value of the fmpz_t to a string.
        //! @param b The base to print the value in.
        std::string toString(const PadicPrintMode& mode) 
        {
            setPrintMode(mode);
            char* str = padic_get_str(nullptr, _val, _ctx);
            return std::string(str);
        }

        ~PadicNumber() 
        {
            padic_clear(_val);
            padic_ctx_clear(_ctx);
        }


        friend std::ostream& operator<<(std::ostream& os, PadicNumber& x)
        {
            os << x.toString(PadicPrintMode::TERSE);
            return os;
        }
        
    };
}


int main() 
{
    flint::Fmpz tmp_a;
    flint::Fmpz tmp_b(static_cast<flint::unsigned_long_t>(10));
    flint::Fmpz tmp_c(static_cast<flint::unsigned_long_t>(1000000000));

    tmp_a.set( static_cast<flint::unsigned_long_t>(1023) );
    tmp_b.set( static_cast<flint::signed_long_t>(-1023) );

    std::cout << "tmp_a: " << tmp_a << " (0b" << tmp_a.toString(flint::Base(2)) << "), is prime: " << tmp_a.isPrime() << std::endl;
    std::cout << "tmp_b: " << tmp_a << " (0b" << tmp_b.toString(flint::Base(2)) << "), is prime: " << tmp_b.isPrime() << std::endl;

    // case 1 (x ≡ 127 mod 7^10)
    flint::Fmpz p;
    p.set( static_cast<flint::unsigned_long_t>(7) );
    std::cout << "p: " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << std::endl;

    flint::PadicNumber padic(p, flint::signed_long_t(10));
    padic.set( static_cast<flint::unsigned_long_t>(127) );

    std::cout << "padic: " << padic << std::endl;

    return 0;
}

/*
int main(void)
{
    fmpz_t p;
    padic_ctx_t ctx;

    char *str;
    padic_t x, y;

    flint_printf("Output:\n\n");

    // Case 1
    flint_printf("Positive integer:  x = 127 mod 7^10\n");

    fmpz_init(p);
    fmpz_set_ui(p, 7);
    padic_ctx_init(ctx, p, 8, 12, PADIC_TERSE);

    padic_init2(x, 10);
    padic_set_ui(x, 127, ctx);

    ctx->mode = PADIC_TERSE;
    str = padic_get_str(NULL, x, ctx);
    flint_printf("print:   "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("get_str: %s\n", str);
    flint_free(str);

    ctx->mode = PADIC_SERIES;
    str = padic_get_str(NULL, x, ctx);
    flint_printf("print:   "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("get_str: %s\n", str);
    flint_free(str);

    ctx->mode = PADIC_VAL_UNIT;
    str = padic_get_str(NULL, x, ctx);
    flint_printf("print:   "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("get_str: %s\n", str);
    flint_free(str);

    padic_clear(x);

    padic_ctx_clear(ctx);
    fmpz_clear(p);

    // Case 2 
    flint_printf("Positive integer larger than p^N:  x = 1057 mod 2^10\n");

    fmpz_init(p);
    fmpz_set_ui(p, 2);
    padic_ctx_init(ctx, p, 10, 12, PADIC_TERSE);

    padic_init2(x, 10);
    padic_set_ui(x, 1057, ctx);

    ctx->mode = PADIC_TERSE;
    str = padic_get_str(NULL, x, ctx);
    flint_printf("print:   "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("get_str: %s\n", str);
    flint_free(str);

    ctx->mode = PADIC_SERIES;
    str = padic_get_str(NULL, x, ctx);
    flint_printf("print:   "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("get_str: %s\n", str);
    flint_free(str);

    ctx->mode = PADIC_VAL_UNIT;
    str = padic_get_str(NULL, x, ctx);
    flint_printf("print:   "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("get_str: %s\n", str);
    flint_free(str);

    padic_clear(x);

    padic_ctx_clear(ctx);
    fmpz_clear(p);

    // Case 3 
    flint_printf("Negative integer:  x = -127 mod 3^10\n");

    fmpz_init(p);
    fmpz_set_ui(p, 3);
    padic_ctx_init(ctx, p, 10, 12, PADIC_TERSE);

    padic_init2(x, 10);
    padic_set_si(x, -127, ctx);

    ctx->mode = PADIC_TERSE;
    str = padic_get_str(NULL, x, ctx);
    flint_printf("print:   "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("get_str: %s\n", str);
    flint_free(str);

    ctx->mode = PADIC_VAL_UNIT;
    str = padic_get_str(NULL, x, ctx);
    flint_printf("print:   "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("get_str: %s\n", str);
    flint_free(str);

    padic_clear(x);

    padic_ctx_clear(ctx);
    fmpz_clear(p);

    // Log 
    flint_printf("Log of 7380996 mod 5^20\n");

    fmpz_init(p);
    fmpz_set_ui(p, 5);
    padic_ctx_init(ctx, p, 10, 25, PADIC_SERIES);

    padic_init(x);
    padic_init(y);
    padic_set_ui(x, 7380996, ctx);

    padic_log(y, x, ctx);

    flint_printf("x = "), padic_print(x, ctx), flint_printf("\n");
    flint_printf("y = "), padic_print(y, ctx), flint_printf("\n");

    padic_clear(x);
    padic_clear(y);

    padic_ctx_clear(ctx);
    fmpz_clear(p);

    return 0;
}
*/