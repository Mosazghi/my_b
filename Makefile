run:
	./build.sh
	./build/my_b --url https://browser.engineering/examples/xiyouji.html
build:
	./build.sh
build-tests:
	cmake --build build --target tests -- -j "$(nproc)"
test:
	ctest --test-dir build --output-on-failure
clean:
	rm -rf build
format:
	find src/ -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i -style=file {} +
check:
	find src/ -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) | xargs clang-tidy -fix -- -std=c++20
