/* Copyright (c) 2014-2015 "Omnidome" by Michael Winkelmann
 * Dome Mapping Projection Software (http://omnido.me).
 * Omnidome was created by Michael Winkelmann aka Wilston Oreo (@WilstonOreo)
 *
 * This file is part of Omnidome.
 *
 * Omnidome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OMNI_TUNINGLIST_H
#define OMNI_TUNINGLIST_H

#include <memory>
#include <vector>
#include <omni/serialization/Interface.h>
#include <omni/proj/Tuning.h>

namespace omni {
  class Session;
  
  namespace proj {
    /**@brief Tuning List contains a list of tunings
     *@detail Tuning List is serializable via QDataStream
     **/
    class OMNI_EXPORT TuningList {
      public:
        TuningList(Session const&);

		TuningList(TuningList const&) = delete;
		TuningList& operator=(TuningList const&) = delete;

		using container_type = std::vector<std::unique_ptr<Tuning>>;

		size_t size() const { return tunings_.size(); }
		bool empty() const { return tunings_.empty(); }

		container_type::iterator begin() { return tunings_.begin(); }
		container_type::iterator end() { return tunings_.end(); }

		container_type::const_iterator begin() const { return tunings_.begin(); }
		container_type::const_iterator end() const { return tunings_.end(); }

        /**@brief Add tuning to list
         * @detail Returns pointer new tuning
         * @param _makeCurrent Flag whether to set current index when added
         *successfully
         **/
        Tuning       * add(bool _makeCurrent = true);

        /// Remove tuning with current index
        void           remove();

        /// Remove tuning with custom index
        void           remove(int);

        /**@brief Returns pointer to current tuning
         * @detail Returns nullptr if currentIdx_ == -1 or tuning list is empty
         **/
        Tuning       * current();

        /**@brief Returns pointer to current tuning (const version)
         * @detail Returns nullptr if currentIdx_ == -1 or tuning list is empty
         **/
        Tuning const * current() const;

        /// Set new current index, must be between 0 and size()-1
        void           setCurrentIndex(int);

        /// Return current index
        int            currentIndex() const;

        /// Deletes all tunings
        void           clear();

        /// Returns tuning at a specific index
        Tuning       * operator[](int);

        /// Returns tuning at a specific index (const version)
        Tuning const * operator[](int) const;

        /// Deserialize list from stream
        void           fromStream(QDataStream&);

        /// Serialize list to stream
        void           toStream(QDataStream&) const;

        /// Test both lists for equality
        friend bool    operator==(TuningList const&,
                                  TuningList const&);

      private:
		  container_type tunings_;

		  bool validIndex(int) const;

		  int currentIdx_ = -1;
		  Session const& session_;
    };
  }
}

OMNI_DECL_STREAM_OPERATORS(omni::proj::TuningList)

#endif /* OMNI_TUNINGLIST_H */
