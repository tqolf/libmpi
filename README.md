# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

| brief | average time<br>(nanoseconds) | instability<br>(coefficient of variation) | rating |
| :-- | :-: | :-: | :-: |
| from-string(mpi vs openssl) | 2443.7<br>30303.4* | 0.0701562 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.4006<br>(Tu es mon meilleur frère...)</span> |
| to-string(mpi vs openssl) | 1328.88<br>3463.21* | 0.109777 | <span style="color:#008000;font-weight:bold;">2.60612<br>(Tu peux faire mieux, continue)</span> |
| from-octets(mpi vs openssl) | 273.632<br>702.13* | 0.0870046 | <span style="color:#008000;font-weight:bold;">2.56597<br>(Tu peux faire mieux, continue)</span> |
| to-octets(mpi vs openssl) | 172.067<br>1475.5* | 0.359989 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">8.57515<br>(C'est super, dessine-toi une tarte)</span> |
| add(mpi vs openssl) | 51.1222<br>333.814* | 0.164442 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.52973<br>(C'est super, dessine-toi une tarte)</span> |
| add-assign(mpi vs openssl) | 56.7424<br>332.054* | 0.202937 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.85196<br>(C'est super, dessine-toi une tarte)</span> |
| sub(mpi vs openssl) | 61.6028<br>162.647* | 0.207007 | <span style="color:#008000;font-weight:bold;">2.64025<br>(Tu peux faire mieux, continue)</span> |
| sub-assign(mpi vs openssl) | 58.2224<br>288.852* | 0.155195 | <span style="color:#008000;font-weight:bold;">4.96119<br>(Tu peux faire mieux, continue)</span> |
| mul(mpi vs openssl) | 2070.41<br>14037.9* | 0.0553581 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.78025<br>(C'est super, dessine-toi une tarte)</span> |
| sqr(mpi vs openssl) | 1329.62<br>8760.12* | 0.168403 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.58845<br>(C'est super, dessine-toi une tarte)</span> |
| MUL2(a * 2 = a + a) | 37.5416 | 0.163214 | <span style="font-style:italic;">N/A</span> |
| MUL2(a * 2 = a << 1) | 77.5234 | 0.113647 | <span style="font-style:italic;">N/A</span> |
