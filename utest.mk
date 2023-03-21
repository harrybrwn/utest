UTEST_DEPS?=
UTEST_TEST_DIR?=tests
UTEST_DIR?=$(UTEST_TEST_DIR)/utest
UTEST_BIN?=$(UTEST_TEST_DIR)/test
UTEST_VERSION?=master

UTEST_TEST_FILES=$(shell find $(UTEST_TEST_DIR) -not -regex '.*$(UTEST_DIR)/.*' -name '*.c')
UTEST_TEST_OBJ=$(patsubst %.c,%.o, $(UTEST_TEST_FILES))
_UTEST_COMPILE_DEPS=$(patsubst %.h,, $(UTEST_DEPS)) $(UTEST_TEST_DIR)/utest.o

test: $(UTEST_BIN)
	@./$(UTEST_BIN)

$(UTEST_BIN): $(_UTEST_COMPILE_DEPS) $(UTEST_TEST_OBJ)
	$(LINK.c) $(OUTPUT_OPTION) $^

$(UTEST_TEST_OBJ): $(UTEST_TEST_FILES) $(UTEST_TEST_DIR)/utest.o

# Build the library
$(UTEST_TEST_DIR)/utest.o: $(UTEST_DIR)/utest.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(UTEST_DIR)/utest.c: utest_install

utest_clean:
	$(RM) $(UTEST_BIN) $(UTEST_TEST_DIR)/*.o

utest_install:
	@if [ ! -d $(UTEST_DIR) ]; then \
		git clone --branch $(UTEST_VERSION) --depth 1 \
			'https://github.com/harrybrwn/utest.git'  \
			$(UTEST_DIR) ;\
	fi

utest_remove:
	$(RM) -r $(UTEST_DIR)

utest_update: utest_remove utest_install

.PHONY: test utest_clean utest_install utest_remove utest_update

# Add utest_clean to clean
clean: utest_clean

# vim: filetype=make noexpandtab
