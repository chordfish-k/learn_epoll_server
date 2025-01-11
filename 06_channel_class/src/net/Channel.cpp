#include "Channel.h"

#include "Epoll.h"

Channel::Channel(Epoll* ep, int fd) 
  : m_Ep(ep), m_Fd(fd){

}

Channel::~Channel() {

}

void Channel::UseET() {
  m_Events |= EPOLLET;
}
  
void Channel::EnableReading() {
  m_Events |= EPOLLIN;
  m_Ep->UpdateChannel(this);
}

void Channel::SetInEpoll() {
  m_InEpoll = true;
}
  
void Channel::SetREvents(uint32_t ev) {
  m_REvents = ev;
}

int Channel::GetFd() const {
  return m_Fd;
}

bool Channel::IsInEpoll() const {
  return m_InEpoll;
}

uint32_t Channel::GetEvents() const {
  return m_Events;
}
  
uint32_t Channel::GetREvents() const {
  return m_REvents;
}
