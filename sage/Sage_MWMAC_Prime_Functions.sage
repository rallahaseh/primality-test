import random
from past.builtins import xrange

# ---------------------------- MRT Algorithm -----------------------------------

def miller_rabin(n, k = 40, isMontgomery = False):

    s = 0
    d = n-1
    prec = n.nbits()
    r = (2^prec) % n
    rinv = inverse_mod(r,n)

    while d & 1 == 0:
        d = d >> 1
        s += 1

    for _ in range(k):
        a = random.randrange(2, n-2)
        if isMontgomery:
            temporary = Prime_MontExp(a,d,n,prec,r,rinv)
            if temporary == r or temporary == n-r:
                continue
                for _ in range(s - 1):
                    temporary = (temporary * temporary) % n
                    if temporary == n - 1:
                        break
            else:
                return False
        else:
            temporary = Prime_ModExp(a,d,n,prec)
            if temporary == 1 or temporary == n-1:
                continue
                for _ in range(s - 1):
                    temporary = (temporary * temporary) % n
                    if temporary == n - 1:
                        break
            else:
                return False
    return True

# Taken from >> CryptoCore_User_Story_2-20211123 (Replacment of Pow(a, d, n))
def Prime_ModExp(b,e,n,prec):
    x = (1) % n
    exp = e
    for i in reversed(range(prec)):
        x = (x * x) % n
        if(Integer(exp).digits(base=2,padto=prec)[i] == 1):
            x = (b * x) % n
    c = x
    return c

# Taken from >> CryptoCore_User_Story_2-20211123 (Replacment of Pow(a, d, n) and integrating the montgomery method)
def Prime_MontExp(b,e,n,prec,r,rinv):
	r2 = (r*r) % n
	x = (1 * r2 * rinv) % n
	exp = e
	for i in reversed(xrange(prec)):
		x = (x * x * rinv) % n
		if(Integer(exp).digits(base=2,padto=prec)[i] == 1):
			x = (b * x * rinv) % n
	c = x
	return(c)

# --------------------------- Prime Generator ----------------------------------

def generate_prime(bits, isMontgomery = False):
    while True:
        random_number = Integer((random.randrange(1 << (bits - 2), 1 << (bits - 1)) << 1) + 1)
        if miller_rabin(random_number):
            return random_number

# ------------------------ Safe Prime Generator --------------------------------

def generate_safe_prime(bits, isMontgomery = False):
    while True:
        random_number = Integer((random.randrange(1 << (bits - 2), 1 << (bits - 1)) << 1) + 1)
        if isMontgomery:
            random_number = random_number | 2^(bits - 1) + 3
        if safe_prime(random_number):
            return random_number

def safe_prime(n):
    if miller_rabin(n):
        d = (n-1) >> 1
        if miller_rabin(d):
            return True
    return False
# -----------------------------------------------------------------------------
