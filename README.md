# Fork, Communication and Bytecoin Mining

We wrote a program to use many processes to mine bytecoin for us!
We'll be rich!

Problem is, we have a performance issue.
We're running the mining in many different processes, and they all finish at different times, but they take forever to get back to us.
We want to get the bytecoins back *as they are mined*, and don't want to wait for the *next* one to cache in.
Currently, it does this:

```
church$ ./bin
Mining 0 done, cha-ching!
Mining 1 done, cha-ching!
Mining 2 done, cha-ching!
Mining 3 done, cha-ching!
Mining 4 done, cha-ching!
Mining 5 done, cha-ching!
Mining 6 done, cha-ching!
Mining 7 done, cha-ching!
Mining 8 done, cha-ching!
Mining 9 done, cha-ching!
Mining 10 done, cha-ching!
Mining 11 done, cha-ching!
Mining 12 done, cha-ching!
Mining 13 done, cha-ching!
Mining 14 done, cha-ching!
Mining 15 done, cha-ching!
```

What we want, with non-blocking communication is something more like this:
```
church$ ./bin
Mining 2 done, cha-ching!
Mining 3 done, cha-ching!
Mining 10 done, cha-ching!
Mining 13 done, cha-ching!
Mining 6 done, cha-ching!
Mining 8 done, cha-ching!
Mining 12 done, cha-ching!
Mining 15 done, cha-ching!
Mining 11 done, cha-ching!
Mining 7 done, cha-ching!
Mining 9 done, cha-ching!
Mining 1 done, cha-ching!
Mining 14 done, cha-ching!
Mining 0 done, cha-ching!
Mining 5 done, cha-ching!
Mining 4 done, cha-ching!
```

- **Question 1:**
	Why does non-blocking approach give you *bursty* responses?
	I.e. a bunch cache in at the same time.
- **Question 2:**
	What impact will allowing the children to cache out in any order have?
- **TODO 1:**
	Change the implementation to use non-blocking communication.
	Please note that there is already a `nbpipe` that has a non-blocking `read` end.
- **Question 3:**
	Is non-blocking faster?
	In what way?
	In what way isn't it faster?
- **TODO 2:**
	We want this all be *very* fast, so we want to avoid the copies of the pipe-based, message-passing implementation.
	Please us the shared memory (already set up in `shmem` for each child) to pass the data back to the parent and cash in.
