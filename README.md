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
|       from-string(mpi vs ossl)        |   1944.86    |         24467.1         |         0.022883         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.5804</span>      |
|        to-string(mpi vs ossl)         |   939.451    |         2207.97         |        0.0327692         |      <span style="color:#008000;font-weight:bold;">2.35027</span>      |
|       from-octets(mpi vs ossl)        |   203.882    |         551.246         |        0.0700414         |      <span style="color:#008000;font-weight:bold;">2.70375</span>      |
|        to-octets(mpi vs ossl)         |   114.261    |         1270.08         |         0.093554         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.1155</span>      |
|           add(mpi vs ossl)            |   68.2408    |         223.263         |         0.121056         |      <span style="color:#008000;font-weight:bold;">3.27169</span>      |
|        add-assign(mpi vs ossl)        |   65.9208    |         225.183         |         0.123167         |      <span style="color:#008000;font-weight:bold;">3.41596</span>      |
|           sub(mpi vs ossl)            |   51.8606    |         146.702         |         0.139808         |      <span style="color:#008000;font-weight:bold;">2.82877</span>      |
|        sub-assign(mpi vs ossl)        |   58.8408    |         226.043         |         0.130881         |      <span style="color:#008000;font-weight:bold;">3.8416</span>       |
|           mul(mpi vs ossl)            |   1625.28    |         9995.08         |        0.0251953         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.14976</span>      |
|           sqr(mpi vs ossl)            |   971.691    |         6946.58         |         0.032229         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.14896</span>      |
|           div(mpi vs ossl)            |   3326.06    |         31187.9         |        0.0177199         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.37683</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.95856e+06  |       1.18755e+07       |        0.00171095        |      <span style="color:#008000;font-weight:bold;">2.99996</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.50019e+06  |       1.05026e+08       |       0.000764213        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.3557</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.18267e+07  |       1.11853e+08       |       0.000750208        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.45766</span>      |
|          MUL2(a * 2 = a + a)          |   63.2408    |      <span style="font-style:italic;">No Reference</span>       |         0.12575          |        N/A        |
|         MUL2(a * 2 = a << 1)          |   81.8208    |      <span style="font-style:italic;">No Reference</span>       |         0.110597         |        N/A        |
