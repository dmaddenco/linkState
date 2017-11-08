all: link

link: manager.cpp manager.h router.cpp router.h logger.cpp logger.h
	g++ -I. -Wall -std=c++11 -g router.cpp -o router
	g++ -I. -Wall -std=c++11 -g manager.cpp -o manager
	g++ -I. -Wall -std=c++11 -g logger.cpp -o logger
router:
	g++ -I. -Wall -std=c++11 -g router.cpp -o router
manager:
	g++ -I. -Wall -std=c++11 -g manager.cpp -o manager
logger:
    g++ -I. -Wall -std=c++11 -g logger.cpp -o logger
clean:
	rm -f *.o *.tar router manager logger
tar:
    tar -cvf link.tar manager.cpp manager.h router.cpp router.h logger.cpp logger.h makefile README.txt