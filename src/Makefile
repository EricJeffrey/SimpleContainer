# do this inside src/ dir!
build: main.cpp
	-mkdir ../build/
	g++ -o ../build/main.out main.cpp -lcap
	echo "BUILD SUCCESS"

.PHONY : cleansu
cleansu:
	# -sudo rm ../build/main.out
	-sudo rm -r ../build/simp_containers/alpine_container
	-sudo rm -r ../build/simp_containers
	echo 'CLEAN SUCCESS'

runsu:
	make cleansu
	make build
	cd ./build && ./main.out
