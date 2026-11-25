# bisect_njit.py
import time
from numba import njit

@njit(cache=True)
def run(arg, eps, outer):
    mid = 0.0
    n = 0
    for _ in range(outer):
        f, xl, xr = 1000.0, 0.0, 3.0
        while f > eps or f < -eps:
            mid = (xl + xr) / 2.0
            f = mid * mid - arg
            if f > 0.0:
                xr = mid
            else:
                xl = mid
            n += 1
    return mid, n

eps = 1e-9

# Warm-up call: the first call triggers JIT compilation, so keep it out of the timing
run(7.0, eps, 1)

t = time.perf_counter()
mid, n = run(7.0, eps, 100_000)
dt = time.perf_counter() - t
print("sqrt(7)=", mid, " eps=", eps, " iter=", n,
      " time:", dt, "s time/iter=", dt / n * 1e9, "ns/iter")
