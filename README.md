# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

|              description              | average time | average time(reference) | coefficient of variation | perfermance ratio |
| :-: | :-: | :-: | :-: | :-: |
|       from-string(mpi vs ossl)        |    2414.5    |         30858.1         |        0.0823392         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.7804</span>      |
|        to-string(mpi vs ossl)         |   665.513    |         3445.69         |        0.0665816         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.1775</span>       |
|       from-octets(mpi vs ossl)        |   234.852    |         721.115         |          0.1679          |      <span style="color:#008000;font-weight:bold;">3.07052</span>      |
|        to-octets(mpi vs ossl)         |   134.807    |         1519.37         |        0.0861383         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.2708</span>      |
|           add(mpi vs ossl)            |   55.4226    |         317.516         |         0.161752         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.72899</span>      |
|        add-assign(mpi vs ossl)        |   51.2226    |         320.436         |         0.139726         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.25575</span>      |
|           sub(mpi vs ossl)            |   64.6032    |         163.488         |         0.124415         |      <span style="color:#008000;font-weight:bold;">2.53065</span>      |
|        sub-assign(mpi vs ossl)        |    60.883    |         322.716         |         0.128174         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.30059</span>      |
|           mul(mpi vs ossl)            |   2092.22    |         11972.8         |         0.224789         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.72254</span>      |
|           sqr(mpi vs ossl)            |   1229.78    |         9249.97         |         0.124671         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.52165</span>      |
|           div(mpi vs ossl)            |   3425.43    |         40810.6         |         0.044964         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.914</span>       |
|      gcd_consttime(mpi vs ossl)       | 5.06931e+06  |       1.53669e+07       |        0.00882843        |      <span style="color:#008000;font-weight:bold;">3.03135</span>      |
|      montgomery-exp(mpi vs ossl)      | 1.05737e+07  |       1.31772e+08       |        0.0422837         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.4622</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.43875e+07  |       1.41059e+08       |        0.0229953         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.80427</span>      |
|          MUL2(a * 2 = a + a)          |   36.0218    |      <span style="font-style:italic;">No Reference</span>       |         0.16662          |        N/A        |
|         MUL2(a * 2 = a << 1)          |    79.824    |      <span style="font-style:italic;">No Reference</span>       |         0.204801         |        N/A        |

<span style="color:#008000;font-weight:bold;text-decoration:blink;">12.7804</span>
