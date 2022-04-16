Varun Jani , vxj5053@psu.edu
Trisha Mandal , tvm5513@psu.edu




@Test1: 2, 100, 8770000
@Test2: 4, 1000, 14010000
@Test3: 4, 100, 13650000
@Test4: 8, 1000, 21900000
@Test5: 8, 10000, 21870000
@Test6: 1, 1000, 61100000
@Test7: 16, 100, 27730000
@Test8: 32, 1000, 29130000
@Test9: 2, 10000, 84600000

For test 1, the time variation was quite interesting. The seconds varied from lowest 8.7 seconds to the highest 9.7 seconds. Initially, it started with 9.1 seconds and then to 9.7 seconds suddenly going down. We guessed it depends on number of mapper threads and how they are utilized when the threads are initialized. An obvious observation was gathered when we tested the second parameters. The user time increased which clearly makes sense as the number of mapper threads was doubled and the buffer size increased as well. So, we decided to test with the same buffer size but the doubled number of mapper threads to understand the difference somewhat. Interestingly enough, the output depends a lot on the n maps more than the buffer size. For test 3, the user time kept on increasing and even reached higher than test 2's highest time. This showed the importance of using a larger buffer size to achieve less user time.

Test 4 made a lot of sense given the first three tests. The time varied, but went from highest almost to lowest. A good comparision for this test would be test 5. The previous and these tests allowed me to conclude that given the same number of mapper threads, an increase in the buffer size resulted in lower user time (almost). And as the number of mapper threads increases, the time difference between each is 6 to 7 seconds meaning a single mapper thread takes around that long. So, we decided to test on n-maps = 1 and as we predicted, it was accurate.

Test 7,8 and 9 were just different paramters we wanted to try out. One of the patterns we noticed was how the first run, second and the third run were in decreasing order. The fourth run suddenly increasing to higher than the first run and the fifth is just random. Also, doubling the number of mapper threads from 16 to 32 didn't increase the user time as much given the buffer size was also increased. Test 9 was just a random test we wanted to see the results of. When compared to test 1, the time didn't make much difference showing that the buffer size is not the most important parameter affecting the time and is only slightly effective.
