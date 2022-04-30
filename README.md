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
|       from-string(mpi vs ossl)        |   2047.35    |         24171.7         |        0.0221528         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.8064</span>      |
|        to-string(mpi vs ossl)         |   577.827    |         2205.91         |        0.0417302         |      <span style="color:#008000;font-weight:bold;">3.81759</span>      |
|       from-octets(mpi vs ossl)        |   183.302    |         549.907         |        0.0738627         |         <span style="color:#008000;font-weight:bold;">3</span>         |
|        to-octets(mpi vs ossl)         |   115.201    |         1273.36         |        0.0931706         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.0533</span>      |
|           add(mpi vs ossl)            |   46.2006    |         222.143         |         0.147128         |      <span style="color:#008000;font-weight:bold;">4.80822</span>      |
|        add-assign(mpi vs ossl)        |   44.5206    |         223.923         |         0.149874         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.02964</span>      |
|           sub(mpi vs ossl)            |   51.1806    |         142.102         |         0.139783         |      <span style="color:#008000;font-weight:bold;">2.77648</span>      |
|        sub-assign(mpi vs ossl)        |   49.0606    |         225.743         |         0.142796         |      <span style="color:#008000;font-weight:bold;">4.60131</span>      |
|           mul(mpi vs ossl)            |    1606.2    |         9958.54         |        0.0250885         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.20006</span>      |
|           sqr(mpi vs ossl)            |   985.192    |         6969.67         |        0.0320908         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.07442</span>      |
|           div(mpi vs ossl)            |   3444.46    |         31381.6         |         0.017469         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.11075</span>      |
|      gcd_consttime(mpi vs ossl)       | 4.29487e+06  |       1.20102e+07       |       0.000596092        |      <span style="color:#008000;font-weight:bold;">2.79642</span>      |
|      montgomery-exp(mpi vs ossl)      |  8.4469e+06  |       1.08735e+08       |        0.00161913        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.8727</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.17732e+07  |       1.16503e+08       |        0.00102002        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.8956</span>       |
|          MUL2(a * 2 = a + a)          |   35.8606    |      <span style="font-style:italic;">No Reference</span>       |         0.166996         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   67.9408    |      <span style="font-style:italic;">No Reference</span>       |         0.121801         |        N/A        |
