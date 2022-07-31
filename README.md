# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

| description | average time | average time(reference) | coefficient of variation | perfermance ratio |
| :-: | :-: | :-: | :-: | :-: |
| from-string(mpi vs openssl) | 2128 | 28309.6 | 0.0273255 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">13.303398<br>(Bro.)</span> |
| to-string(mpi vs openssl) | 1093.75 | 2908.26 | 0.0853633 | <span style="color:#008000;font-weight:bold;">2.658974<br>(You can do better)</span> |
| from-octets(mpi vs openssl) | 236.071 | 763.016 | 0.0663155 | <span style="color:#008000;font-weight:bold;">3.232144<br>(You can do better)</span> |
| to-octets(mpi vs openssl) | 105.605 | 1496.49 | 0.0973138 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">14.170643<br>(Bro.)</span> |
| add(mpi vs openssl) | 39.9018 | 290.954 | 0.158324 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.291746<br>(Great work)</span> |
| mpi:add-assign | 40.562 | <span style="font-style:italic;">No Reference</span> | 0.157035 | <span style="color:#008000;font-weight:bold;">N/A</span> |
| sub(mpi vs openssl) | 47.8424 | 167.408 | 0.144603 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">3.499156<br>(You can do better)</span> |
| sub-assign(mpi vs openssl) | 47.8224 | 294.854 | 0.144638 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.165600<br>(Great work)</span> |
| mul(mpi vs openssl) | 1897.79 | 13016.1 | 0.0230223 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.858544<br>(Great work)</span> |
| sqr(mpi vs openssl) | 1064.93 | 8513.4 | 0.0307258 | 7.994329<br>(Great work) |
| MUL2(a * 2 = a + a) | 33.2214 | <span style="font-style:italic;">No Reference</span> | 0.173501 | N/A |
| MUL2(a * 2 = a << 1) | 71.2834 | <span style="font-style:italic;">No Reference</span> | 0.11855 | N/A |
