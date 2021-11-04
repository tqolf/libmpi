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
|       from-string(mpi vs ossl)        |   2072.59    |         28428.5         |         0.100857         |      <span style="color:#8000;">**13.7164**</span>      |
|        to-string(mpi vs ossl)         |   622.022    |         2835.75         |         0.144683         |      <span style="color:#8000;">**4.55892**</span>      |
|       from-octets(mpi vs ossl)        |   211.661    |         594.122         |         0.258076         |      <span style="color:#8000;">**2.80696**</span>      |
|        to-octets(mpi vs ossl)         |   140.861    |         1398.82         |         0.35184          |      <span style="color:#8000;">**9.93056**</span>      |
|           add(mpi vs ossl)            |   68.2802    |         231.317         |         0.626962         |      <span style="color:#8000;">**3.38775**</span>      |
|        add-assign(mpi vs ossl)        |   72.3602    |         282.861         |         0.778277         |      <span style="color:#8000;">**3.90907**</span>      |
|           sub(mpi vs ossl)            |   77.0802    |          133.5          |         0.637021         |      <span style="color:#8000;">1.73197</span>      |
|        sub-assign(mpi vs ossl)        |   76.3402    |         233.021         |         0.669797         |      <span style="color:#8000;">**3.0524**</span>       |
|           mul(mpi vs ossl)            |   2332.73    |         10075.6         |        0.0442213         |      <span style="color:#8000;">**4.31922**</span>      |
|           sqr(mpi vs ossl)            |   1149.98    |         8517.66         |        0.0884791         |      <span style="color:#8000;">**7.40677**</span>      |
|           div(mpi vs ossl)            |   3397.75    |         40105.6         |        0.0468007         |      <span style="color:#8000;">**11.8036**</span>      |
|      gcd_consttime(mpi vs ossl)       | 4.34509e+06  |       1.36845e+07       |         0.057173         |      <span style="color:#8000;">**3.14943**</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.40429e+06  |       1.09783e+08       |        0.0474716         |      <span style="color:#8000;">**13.0628**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.17876e+07  |       1.16998e+08       |        0.0678267         |      <span style="color:#8000;">**9.9255**</span>       |
|          MUL2(a * 2 = a + a)          |   34.9202    |      <span style="font-style:italic;">No Reference</span>       |         0.264024         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   69.6804    |      <span style="font-style:italic;">No Reference</span>       |         0.155516         |        N/A        |
