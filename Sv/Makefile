# ============================
#   설정
# ============================
CROSS_COMPILE = aarch64-none-linux-gnu-
CXX = $(CROSS_COMPILE)g++

CXXFLAGS = -Wall -Wextra -std=c++17 -g
TARGET = main

# ============================
#   소스 / 헤더 / 오브젝트
# ============================
SRCS = main.cpp Sv.cpp Wk.cpp
OBJS = $(SRCS:.cpp=.o)

DEPS = Sv.h Wk.h Support.h

# ============================
#   빌드 규칙
# ============================
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ============================
#   확인
# ============================
check:
	file $(TARGET)

# ============================
#   clean
# ============================
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean check
