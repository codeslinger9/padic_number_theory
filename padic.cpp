
/** 
 *   Demo FLINT program to demonstrate some use of the padic module.
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
        PadicContext(const Fmpz& p, signed_long_t min = 8, signed_long_t max = 12) 
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


    public:

        //! @brief Default constructor.
        //PadicNumber(const padic_t& val)

        //! @brief Constructor.
        //! @param prec The precision to use (default is PADIC_DEFAULT_PREC = 20).
        PadicNumber(std::shared_ptr<PadicContext> ctx, signed_long_t prec = PADIC_DEFAULT_PREC) : _ctx(ctx)
        {
            padic_init2(_val, prec);
        }

        //! @brief Set the value of the padic_t to an unsigned long.
        //! @param val The value to set the padic_t to.
        void set(const unsigned_long_t val) 
        {
            padic_set_ui(_val, val, _ctx.get()->get());
        }

        //! @brief Set the value of the padic_t to a signed long.
        //! @param val The value to set the padic_t to.
        void set(const signed_long_t val) 
        {
            padic_set_si(_val, val, _ctx.get()->get());
        }
        
        //! @brief Print the value of the fmpz_t to a string.
        //! @param b The base to print the value in.
        std::string toString(const PadicPrintMode& mode) 
        {
            _ctx->setPrintMode(mode);
            char* str = padic_get_str(nullptr, _val, _ctx.get()->get());
            return std::string(str);
        }

        ~PadicNumber() 
        {
            padic_clear(_val);
        }


        friend std::ostream& operator<<(std::ostream& os, PadicNumber& x)
        {
            os << x.toString(PadicPrintMode::TERSE);
            return os;
        }
        
    };

    // PadicNumber log(const PadicNumber& x) 
    // {
    //     PadicNumber y(x);
    //     padic_log(y._val, x._val, y._ctx);
    //     return y;
    // }
}


int main() 
{
    {
        flint::Fmpz tmp_a;
        flint::Fmpz tmp_b(static_cast<flint::unsigned_long_t>(10));
        flint::Fmpz tmp_c(static_cast<flint::unsigned_long_t>(1000000000));

        tmp_a.set( static_cast<flint::unsigned_long_t>(1023) );
        tmp_b.set( static_cast<flint::signed_long_t>(-1023) );

        std::cout << "tmp_a: " << tmp_a << " (0b" << tmp_a.toString(flint::Base(2)) << "), is prime: " << tmp_a.isPrime() << std::endl;
        std::cout << "tmp_b: " << tmp_a << " (0b" << tmp_b.toString(flint::Base(2)) << "), is prime: " << tmp_b.isPrime() << std::endl;
        std::cout << "\n";
    }


    // case 1 (x ≡ 127 mod 7^10)
    {
        std::cout << "x ≡ 127 mod 7^10" << std::endl;

        flint::Fmpz p;
        p.set( static_cast<flint::unsigned_long_t>(7) );
        std::cout << "p: " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << std::endl;

        auto ctx = std::make_shared<flint::PadicContext>(p);

        flint::PadicNumber padic(ctx, flint::signed_long_t(10));
        padic.set( static_cast<flint::unsigned_long_t>(127) );
        std::cout << "padic: " << padic << " (" << padic.toString(flint::PadicPrintMode::SERIES) << ")" << std::endl;
        std::cout << "\n";
    }

    // case 2 (x ≡ 1057 mod 2^10)
    {   
        std::cout << "x ≡ 1057 mod 2^10" << std::endl;

        flint::Fmpz p;
        p.set( static_cast<flint::unsigned_long_t>(2) );
        std::cout << "p: " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << std::endl;

        auto ctx = std::make_shared<flint::PadicContext>(p);

        flint::PadicNumber padic(ctx, flint::signed_long_t(10));
        padic.set( static_cast<flint::unsigned_long_t>(1057) );
        std::cout << "padic: " << padic << " (" << padic.toString(flint::PadicPrintMode::SERIES) << ")" << std::endl;
        std::cout << "\n";
    }

    // case 3 (x ≡ -127 mod 3^10)
    {
        std::cout << "x ≡ -127 mod 3^10" << std::endl;

        flint::Fmpz p;
        p.set( static_cast<flint::unsigned_long_t>(3) );
        std::cout << "p: " << p << " (0b" << p.toString(flint::Base(2)) << "), is prime: " << p.isPrime() << std::endl;

        auto ctx = std::make_shared<flint::PadicContext>(p);

        flint::PadicNumber padic(ctx, flint::signed_long_t(10));
        padic.set( static_cast<flint::signed_long_t>(-127) );
        std::cout << "padic: " << padic << " (" << padic.toString(flint::PadicPrintMode::SERIES) << ")" << std::endl;
    }

    return 0;
}
