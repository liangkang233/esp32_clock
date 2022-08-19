package main

import (
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"time"

	"github.com/shirou/gopsutil/cpu"
	"github.com/shirou/gopsutil/disk"
	"github.com/shirou/gopsutil/host"
	"github.com/shirou/gopsutil/mem"
	"github.com/shirou/gopsutil/net"
)

var (
	esp32host string = "192.168.253.7"
	// esp32host string = "10.16.100.26"
	Info    *log.Logger
	Warning *log.Logger
	Error   *log.Logger
)

func collet() {
	v, _ := mem.VirtualMemory()
	c, _ := cpu.Info()
	cc, _ := cpu.Percent(time.Second, false)
	d, _ := disk.Usage("/")
	n, _ := host.Info()
	nv, _ := net.IOCounters(true)
	boottime, _ := host.BootTime()
	btime := time.Unix(int64(boottime), 0).Format("2006-01-02 15:04:05")

	Info.Printf("        Mem       : %v MB  Free: %v MB Used:%v Usage:%f%%\n", v.Total/1024/1024, v.Available/1024/1024, v.Used/1024/1024, v.UsedPercent)
	if len(c) > 1 {
		for _, sub_cpu := range c {
			modelname := sub_cpu.ModelName
			cores := sub_cpu.Cores
			Info.Printf("        CPU       : %v   %v cores \n", modelname, cores)
		}
	} else {
		sub_cpu := c[0]
		modelname := sub_cpu.ModelName
		cores := sub_cpu.Cores
		Info.Printf("        CPU       : %v   %v cores \n", modelname, cores)

	}
	Info.Printf("        Network: %v bytes / %v bytes\n", nv[0].BytesRecv, nv[0].BytesSent)
	Info.Printf("        SystemBoot:%v\n", btime)
	Info.Printf("        CPU Used    : used %f%% \n", cc[0])
	Info.Printf("        HD        : %v GB  Free: %v GB Usage:%f%%\n", d.Total/1024/1024/1024, d.Free/1024/1024/1024, d.UsedPercent)
	Info.Printf("        OS        : %v(%v)   %v  \n", n.Platform, n.PlatformFamily, n.PlatformVersion)
	Info.Printf("        Hostname  : %v  \n", n.Hostname)
}

func init() {
	errFile, err := os.OpenFile("errors.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
	if err != nil {
		log.Fatalln("打开日志文件失败：", err)
	}

	Info = log.New(os.Stdout, "Info:", log.Ldate|log.Ltime|log.Lshortfile)
	Warning = log.New(os.Stdout, "Warning:", log.Ldate|log.Ltime|log.Lshortfile)
	Error = log.New(io.MultiWriter(os.Stderr, errFile), "Error:", log.Ldate|log.Ltime|log.Lshortfile)
}

func main() {
	collet()
	if len(os.Args) > 1 {
		esp32host = os.Args[1]
	}

	for {
		time.Sleep(time.Second)
		load := ""
		v, _ := mem.VirtualMemory()
		cc, _ := cpu.Percent(time.Second, false)
		n, _ := host.Info()
		// nv, _ := net.IOCounters(true)
		load += fmt.Sprintf("cpuinfo=%f&", cc[0])
		load += fmt.Sprintf("System_process=%d&", n.Procs)
		load += fmt.Sprintf("memInfo=%f&freemem=%.3f", v.UsedPercent, float64(v.Available)/1024.0/1024.0/1024.0)
		// load += fmt.Sprintf("recv=%v&send=%v", nv[0].BytesRecv, nv[0].BytesSent)

		Info.Println(load)
		resp, err := http.Get(fmt.Sprintf("http://%s/update/?%s", esp32host, load))
		// defer resp.Body.Close()
		if err != nil {
			// Error.Println(err)
			continue
		}
		_, err = ioutil.ReadAll(resp.Body)
		if err != nil {
			Error.Println(err)
			continue
		}
	}
}
