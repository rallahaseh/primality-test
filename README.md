# Ausgewählte Themen der Embedded Software Entwicklung II - Selected Topics of Embedded Software Development II

**Semester:** WS-2021/22

**Instructor:** Prof. Dr. Martin Schramm

**Project:** Testing and generating Prime Numbers and Safe Primes using CryptoCore

[![Foo](https://images2.alphacoders.com/304/thumbbig-3042.webp)]
 
# Group 2 – Team 4

- Rashed Al-Lahaseh – 00821573
- Vikas Gunti - 12100861
- Supriya Kajla – 12100592
- Srijith Krishnan – 22107597
- Wannakuwa Nadeesh – 22109097

# Running the application
### \driver
- `make clean && make`
- `insmod cryptocore_driver.ko`
- `lsmod` to make sure the driver has been loaded

### \application
- `gcc -c main.c`
- `gcc -c prime_Generator.c`
- `gcc -c prime_Tester.c`
- `gcc -c safeprime_Generator.c`
- `gcc -c safeprime_Tester.c`
- `gcc -o combined main.o prime_Generator.o prime_Tester.o safeprime_Generator.o safeprime_Tester.o -lrt`
- `./combined`
