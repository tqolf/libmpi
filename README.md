# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

|              description              | average time(nanoseconds) |  reference   | coefficient of variation | perfermance ratio |
| :-: | :-: | :-: | :-: | :-: |
|       from-string(mpi vs ossl)        |          2083.23          |   24011.9    |        0.0219807         |      <span style="color:#8000;">**11.5263**</span>      |
|        to-string(mpi vs ossl)         |          579.264          |   2245.41    |        0.0415509         |      <span style="color:#8000;">**3.87632**</span>      |
|       from-octets(mpi vs ossl)        |          191.961          |   550.344    |        0.0728518         |      <span style="color:#8000;">**2.86695**</span>      |
|        to-octets(mpi vs ossl)         |          119.021          |   1272.29    |        0.0916625         |      <span style="color:#8000;">**10.6896**</span>      |
|           add(mpi vs ossl)            |          43.8404          |   223.001    |         0.151034         |      <span style="color:#8000;">**5.08666**</span>      |
|        add-assign(mpi vs ossl)        |          46.4604          |   225.042    |         0.146739         |      <span style="color:#8000;">**4.84373**</span>      |
|           sub(mpi vs ossl)            |          50.0804          |   142.501    |         0.141361         |      <span style="color:#8000;">**2.84544**</span>      |
|        sub-assign(mpi vs ossl)        |          50.8004          |   224.742    |         0.148053         |      <span style="color:#8000;">**4.42401**</span>      |
|           mul(mpi vs ossl)            |          1924.01          |   10193.9    |        0.0246932         |      <span style="color:#8000;">**5.29823**</span>      |
|           sqr(mpi vs ossl)            |          992.846          |   6912.99    |        0.0318386         |      <span style="color:#8000;">**6.96279**</span>      |
|           div(mpi vs ossl)            |          3401.74          |   31224.6    |        0.0172494         |      <span style="color:#8000;">**9.17902**</span>      |
|      gcd_consttime(mpi vs ossl)       |        4.72554e+06        | 1.19361e+07  |        0.00113936        |      <span style="color:#8000;">**2.52588**</span>      |
|      montgomery-exp(mpi vs ossl)      |        8.85106e+06        | 1.05627e+08  |       0.000410361        |      <span style="color:#8000;">**11.9339**</span>      |
| montgomery-exp-consttime(mpi vs ossl) |        1.23564e+07        | 1.12928e+08  |       0.000601445        |      <span style="color:#8000;">**9.13921**</span>      |
|          <span style="font-style:italic;">MUL2(a * 2 = a + a)</span>          |           <span style="font-style:italic;">31.52</span>           | <span style="font-style:italic;">No Reference</span> |         <span style="font-style:italic;">0.178122</span>         |        <span style="font-style:italic;">N/A</span>        |
|         <span style="font-style:italic;">MUL2(a * 2 = a << 1)</span>          |          <span style="font-style:italic;">61.4602</span>          | <span style="font-style:italic;">No Reference</span> |         <span style="font-style:italic;">0.127576</span>         |        <span style="font-style:italic;">N/A</span>        |
