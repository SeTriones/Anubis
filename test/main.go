package main

import (
	"net"
	"time"
	"fmt"
	"os"
	"encoding/binary"
)

func main() {
	conn, err := net.DialTimeout("tcp", "127.0.0.1:4444", time.Millisecond*time.Duration(10))
	if err != nil {
		fmt.Fprintf(os.Stderr, "create conn err=%v\n", err)
		return
	}
	key := "testkey-test"
	keylen := len([]byte(key))
	buf := make([]byte, 12 + keylen)
	binary.BigEndian.PutUint32(buf[0:4], uint32(4 + keylen))
	binary.BigEndian.PutUint32(buf[4:8], uint32(0))
	binary.BigEndian.PutUint32(buf[8:12], uint32(10))
	cnt := copy(buf[12:], []byte(key))
	if cnt != keylen {
		fmt.Fprintf(os.Stderr, "copy fail,cnt=%d\n", cnt)
		return
	}
	n, err := conn.Write(buf)
	if err != nil {
		fmt.Fprintf(os.Stderr, "write err=%v,n=%d\n", err, n)
	} else {
		fmt.Fprintf(os.Stderr, "write succn=%d\n", n)
	}
	rbuf := make([]byte, 5)
	conn.SetReadDeadline(time.Now().Add(time.Millisecond*time.Duration(10)))
	idx := 0
	for {
		n, err = conn.Read(rbuf[idx:])
		if err != nil {
			break
		} else {
			idx = idx + n
			if idx == 5 {
				break
			}
		}
	}
	if err != nil {
		fmt.Fprintf(os.Stderr, "read err=%v,n=%d\n", err, n)
	} else {
		fmt.Fprintf(os.Stderr, "read succ,n=%d\n", n)
		for i := 0; i < 5; i++ {
			fmt.Fprintf(os.Stderr, "recvbuf[%d]=%v\n", i, uint8(rbuf[i]))
		}
		endian := uint8(rbuf[0])
		var tmp uint32
		if endian == 0 {
			fmt.Fprintf(os.Stderr, "small endian\n")
			// small endian
			tmp = binary.LittleEndian.Uint32(rbuf[1:])
		} else {
			fmt.Fprintf(os.Stderr, "big endian\n")
			// bid endian
			tmp = binary.BigEndian.Uint32(rbuf[1:])
		}
		fmt.Fprintf(os.Stderr, "cnt=%d\n", tmp)
	}

	return
}
