CXX ?= g++
GENERATE_22M="for i in $$(seq 1 1000000); do echo "1234567890 line $$i"; done"

.PHONY: cleanup test_simple test_compress

logrotee: logrotee.cpp
	$(CXX) logrotee.cpp -o logrotee

cleanup:
	rm -f /tmp/logrotee-test*

test_simple: cleanup
	bash -c 'for i in $$(seq 1 1000000); do echo "1234567890 line $$i"; done' | \
		cmake-build-debug/logrotee --null /tmp/logrotee-test
	ls -lah /tmp/logrotee-test*

test_compress: cleanup
	bash -c 'for i in $$(seq 1 1000000); do echo "1234567890 line $$i"; done' | \
	cmake-build-debug/logrotee \
		--null --compress "bzip2 {}" --compress-suffix .bz2 \
		--chunk 20000 /tmp/logrotee-test
	ls -lah /tmp/logrotee-test*
