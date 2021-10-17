# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

|     operation with options     | average time(nanoseconds) | coefficient of variation | perfermance ratio to ref |
| :-: | :-: | :-: | :-: |
| from-string(ossl)              |       27759.546600        |         0.076081         |         1.000000         |
| to-string(ossl)                |        2658.803400        |         0.103056         |         1.000000         |
| from-string(mpi)               |        2029.147400        |         0.115457         |        <span style="color:#8000;">**13.680399**</span>         |
| to-string(mpi)                 |        619.876200         |         0.149194         |         <span style="color:#8000;">**4.289249**</span>         |
| from-octets(ossl)              |        696.435800         |         0.136046         |         1.000000         |
| to-octets(ossl)                |        1280.732000        |         0.102582         |         1.000000         |
| from-octets(mpi)               |        197.078600         |         0.144335         |         <span style="color:#8000;">**3.533797**</span>         |
| to-octets(mpi)                 |        132.659200         |         0.256407         |         <span style="color:#8000;">**9.654302**</span>         |
| add(ossl)                      |        307.558000         |         0.177036         |         1.000000         |
| add(mpi)                       |         44.579800         |         0.218792         |         <span style="color:#8000;">**6.899044**</span>         |
| add-assign(ossl)               |        275.938200         |         0.105264         |         1.000000         |
| add-assign(mpi)                |         46.739800         |         0.207516         |         <span style="color:#8000;">**5.903709**</span>         |
| sub(ossl)                      |        136.699000         |         0.102787         |         1.000000         |
| sub(mpi)                       |         79.519400         |         0.192052         |         <span style="color:#8000;">1.719065</span>         |
| sub-assign(ossl)               |        284.758200         |         0.292802         |         1.000000         |
| sub-assign(mpi)                |         65.299600         |         0.233895         |         <span style="color:#8000;">**4.360795**</span>         |
| mul(ossl)                      |        9627.720000        |         0.040637         |         1.000000         |
| mul(mpi)                       |        1823.248600        |         0.137319         |         <span style="color:#8000;">**5.280531**</span>         |
| sqr(ossl)                      |        6627.278600        |         0.031094         |         1.000000         |
| sqr(mpi)                       |        951.654200         |         0.061358         |         <span style="color:#8000;">**6.963957**</span>         |
| div(ossl)                      |       35298.439600        |         0.059423         |         1.000000         |
| div(mpi)                       |        4549.931600        |         0.258402         |         <span style="color:#8000;">**7.758015**</span>         |
| gcd_consttime(ossl)            |      14162025.857143      |         0.074850         |         1.000000         |
| gcd_consttime(mpi)             |      3990194.217391       |         0.073981         |         <span style="color:#8000;">**3.549207**</span>         |
| montgomery-exp(ossl)           |     112040660.400000      |         0.047004         |         1.000000         |
| montgomery-exp-consttime(ossl) |     114681283.800000      |         0.039686         |         1.000000         |
| montgomery-exp(mpi)            |      9249293.145455       |         0.046808         |        <span style="color:#8000;">**12.113429**</span>         |
| montgomery-exp-consttime(mpi)  |      12743743.285714      |         0.036066         |         <span style="color:#8000;">**8.999027**</span>         |
| generate_prime(ossl)           |    14992230215.000000     |         0.532289         |         1.000000         |
| is_prime(ossl)                 |     484715501.000000      |         0.012196         |         1.000000         |
| MUL2(a * 2 = a + a)            |         30.159800         |         0.182109         |         1.000000         |
| MUL2(a * 2 = a << 1)           |         63.619600         |         0.162073         |         <span style="color:#ff0000;">**0.474065**</span>         |

