// index.ts
// 获取应用实例
Page({
  data: {
    countdownText: '剩余00天00小时00分00秒',
    countdownInterval: null,
    targetTime: null,
  },
  handleInput(e) {
    this.setData({
      targetTime: e.detail.value,
    });
  },
  startCountdown() {
    if (this.data.targetTime) {
      const targetTime = this.data.targetTime;
      const now = new Date();
      const target = new Date(
        targetTime.slice(0, 4),
        parseInt(targetTime.slice(4, 6)) - 1,
        targetTime.slice(6, 8)
      );
      const diff = target - now;
      if (diff > 0) {
        this.updateCountdown(diff);
        this.data.countdownInterval = setInterval(() => {
          const diff = target - new Date();
          if (diff > 0) {
            this.updateCountdown(diff);
          } else {
            this.stopCountdown();
          }
        }, 1000);
      } else {
        wx.showToast({
          title: '请输入正确的时间',
          icon: 'none',
        });
      }
    } else {
      wx.showToast({
        title: '请输入时间',
        icon: 'none',
      });
    }
  },
  stopCountdown() {
    clearInterval(this.data.countdownInterval);
    this.setData({
      countdownText: '剩余00天00小时00分00秒',
      countdownInterval: null,
      targetTime: null,
    });
  },
  updateCountdown(diff) {
    const days = Math.floor(diff / (1000 * 60 * 60 * 24));
    const hours = Math.floor((diff % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60));
    const minutes = Math.floor((diff % (1000 * 60 * 60)) / (1000 * 60));
    const seconds = Math.floor((diff % (1000 * 60)) / 1000);
    const countdownText = `剩余${this.formatTime(days)}天${this.formatTime(hours)}小时${this.formatTime(
      minutes
    )}分${this.formatTime(seconds)}秒`;
    this.setData({
      countdownText,
    });
  },
  formatTime(time) {
    return time < 10 ? `0${time}` : time;
  },
  sendData() {
    var data=this.data.targetTime;
    console.log(data)
    data=JSON.stringify(data)
    wx.request({
      url: 'http://192.168.22.82:80',
      method:'POST',
      data:{command:data},
      
    })
    wx.showToast({
      title: '发送数据成功',
      icon: 'success',
    });
    fail:(error)=>{
      console.log(error);
      wx.showToast({
        title:'数据发送失败',
        icon:'none',
      })
    }
  },
});