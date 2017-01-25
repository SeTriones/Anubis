# :=是赋值。?=的意思是如果上层传递了这个变量，就用上层的，否则使用表达式的

TARGET := Anubis

DISTRIB_ROOT ?= ..

EXCLUDE_SUBDIRS :=
# shell是执行shell命令
SUBDIRS := $(shell find . -maxdepth 1 -type d)
# basename, notdir是makefile的内置函数
# notdir把所有子目录前缀都去掉，比如./test，就变成test
# basename把所有参数的值在最后一个.之前的部分都去掉，这样就去掉了当前目录下的隐藏目录
SUBDIRS := $(basename $(notdir $(SUBDIRS)))
# filter-out根据名字就能知道是去掉一些预制的不需要的字符
SUBDIRS := $(filter-out $(EXCLUDE_SUBDIRS),$(SUBDIRS))

DEPS_DIR := .deps

# wildcard是匹配通配符，这里会把当前目录下的所有*.cc *.c *.cpp都赋值给SOURCES这个变量
SOURCES := $(wildcard *.cc *.c *.cpp)
# patsubst是替换通配符，把所有的*.c变成*.o
OBJS := $(patsubst %.c, %.o, $(SOURCES))
OBJS := $(patsubst %.cc, %.o, $(OBJS))
OBJS := $(patsubst %.cpp, %.o, $(OBJS))
DEPS := $(patsubst %.o, %.d, $(OBJS))
# addprefix意思就是它的名字，添加前缀
DEPS_FP := $(addprefix $(DEPS_DIR)/, $(DEPS))
DEPS_FP_EXIST := $(wildcard $(DEPS_DIR)/*.d)
DEPS_FP_MISSING := $(filter-out $(DEPS_FP_EXIST), $(DEPS_FP))
# 这里有一个比较奇怪的地方，这些.d文件是干什么的呢？我们会在后面解释

RM-RF := rm -rf
CC := gcc
CXX := g++ -std=c++1y
INCLUDES := -I. 
CPPFLAGS := 
CFLAGS := -g -O2 -Wall -fPIC
CXXFLAGS := -g -O2 -Wall -fPIC
LFLAGS :=
LDFLAGS := -lpthread -lcrypto
LIBS :=

.PHONY : all objs dirs clean clean-objs clean-deps subdirs subdirs_clean
all : dirs $(TARGET)

# 这堆$开头的东西很奇怪，简单起见，我们只需要知道$@是目标集合，$^是依赖集合，$^是依赖中第一个目标的名字
# 假设下面的这个语句，TARGET是test，OBJS是main.o test.o，那么
# $@就是test，$^就是main.o test.o，$^就是main.o

$(TARGET) : $(OBJS) $(LIBS)
	$(CXX) -o $@ $^ $(LDFLAGS) 

objs : dirs $(OBJS)

dirs :
ifeq ($(wildcard $(DEPS_DIR)),)
	@mkdir -p $(DEPS_DIR)
endif

subdirs :
	@for subdir in $(SUBDIRS); \
		do \
			cd $$subdir && make; \
		done

subdirs_clean :
	@for subdir in $(SUBDIRS); \
		do \
			cd $$subdir && make clean; \
		done

.cc.o :
	$(CXX) $(CPPFLAGS) $(INCLUDES) -MMD -MP -MF $(DEPS_DIR)/$(*F).d -c $< -o $@

.cpp.o :
	$(CXX) $(CPPFLAGS) $(INCLUDES) -MMD -MP -MF $(DEPS_DIR)/$(*F).d -c $< -o $@

# $$(*F)取的是$@的文件部分，假设$@是/a/b/test.cpp，那么$(*F)就是test
.c.o :
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(DEPS_DIR)/$(*F).d -c $< -o $@

clean-objs :
	$(RM-RF) $(OBJS)

clean-deps :
	$(RM-RF) $(DEPS_DIR)

clean : subdirs_clean clean-objs clean-deps
	$(RM-RF) $(TARGET)

ifneq ($(DEPS_FP_MISSING),)
$(DEPS_FP_MISSING):
	@$(RM-RF) $(patsubst %.d, %.o, $(@F))
endif

# -include是引用其他依赖描述文件。其实这些依赖我们都保存在.deps/*.d里面了
#  你可以通过g++的 -MMD -MP -MF来生成对应的.d，会把c++/c文件的头文件依赖都写到对应的.d文件里
#  这样就不用手动在Makefile里加上对于头文件的依赖关系了
#  include前面加上-的意思是即使出现错误也要继续执行
-include $(DEPS_FP)
