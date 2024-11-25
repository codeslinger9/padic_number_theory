
// FLINT C++ wrapper

#include <gmp.h>
#include <flint/flint.h>
#include <flint/fmpz.h>
#include <flint/fmpz_mod_poly.h>
#include <flint/aprcl.h>
#include <flint/padic.h>


#include <memory>
#include <string>
#include <stdexcept>

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
        friend PadicNumber operator - (const PadicNumber& lhs, const PadicNumber& rhs);

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

    PadicNumber operator - (const PadicNumber& lhs, const PadicNumber& rhs) 
    {
        PadicNumber y(lhs.getContext());
        padic_sub(y._val, lhs._val, rhs._val, lhs._getContext());
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