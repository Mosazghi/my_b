run:
	./build.sh
	./build/my_b --url https://browser.engineering/examples/xiyouji.html
build:
	./build.sh
test:
	ctest --test-dir build --output-on-failure
clean:
	rm -rf build
