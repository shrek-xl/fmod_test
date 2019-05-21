# fmod_test
simple test on what should be done to use multithreading in FMOD

there is a channel array which have certain amount of channels.

Two threads（main and sub thread）will get free channel from array to play its own sound.

I read the FMOD doc, it says that FMOD now is not thread safe.(FMOD ex)

So I put mutex when using every function which need FMOD::System instance.

I suppose FMOD commands will be submitted to that FMOD::System instance, so better make sure it can be thread safe.

I have run my program for maybe half hours, it hasn't crashed yet. So it should be fine.

BTW the code is not well organized, just show some ideas.

like what I said before, it's simple test, but can be useful.
