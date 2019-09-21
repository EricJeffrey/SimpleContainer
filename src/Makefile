build : main.cc
	g++ -o ../build/main.out main.cc

.PHONY : clean
clean :
	-rm ../build/main.out
	-rm -r ../simp_container_root

run :
	make clean
	make build
	../build/main.out

.PHONY : cleansu
cleansu:
	-sudo rm ../build/main.out
	-sudo rm -r ../simp_container_root

runsu: 
	make cleansu
	make build
	sudo ../build/main.out
