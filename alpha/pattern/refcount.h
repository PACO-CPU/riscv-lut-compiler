/*
  Copyright 2014 Peter Wagener
  
  This file is part of the alpha framework.
  
  The alpha framework is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  The alpha framework is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this code.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ALPHA_PATTERN_REFCOUNT_H
#define _ALPHA_PATTERN_REFCOUNT_H

#include <exception>

namespace alp {

class RefcountDestructException : public std::exception {
  virtual const char *what() const throw() {
    return "Refcount imbalance: destroyed object with non-zero counter!";
  }
};

extern RefcountDestructException refcountDestructException;

class RefcountDropException : public std::exception {
  virtual const char *what() const throw() {
    return "Refcount imbalance: dropped object with non-positive counter!";
  }
};

extern RefcountDropException refcountDropException;

class IRCObject {
  public:
    virtual ~IRCObject() { };
    virtual bool drop()=0;
    
    virtual void grab()=0;
};

/** \brief Refcounted object.
  * 
  * Inheriting objects should never be destroyed directly. Instead
  * any instance using a refcounted object must grab it after creation
  * or acquisition and drop it after it is no longer used.
  */
class RCObject {
  private:
    int _refcount;
  public:
    RCObject();
    
    virtual ~RCObject();
    /** \brief Decrements the reference counter.
      *
      * After calling this method, the callee must not use the object
      * reference again as it is deleted if the counter hits zero.
      * \return True iff the object was destroyed.
      */
    bool drop();
    
    /** \brief Increments the reference counter.
      *
      * This call must eventually be accompanied with a call to drop.
      */
    void grab();
    
    /** \brief Returns the current reference counter value.
      */
    int refcount();
};

class RCObjectv : public IRCObject {
  private:
    int _refcount;
  public:
    RCObjectv();
    
    virtual ~RCObjectv();
    virtual bool drop();
    virtual void grab();
    int refcount();
};

} // namespace alp


#endif
