/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * *
 * MeetiX OS By MeetiX OS Project [Marco Cicognani]                                    *
 *                                                                                     *
 *         DERIVED FROM THE GHOST OPERATING SYSTEM                                     *
 *         This software is derived from the Ghost operating system project,           *
 *         written by Max Schlüssel <lokoxe@gmail.com>. Copyright 2012-2017            *
 *         https://ghostkernel.org/                                                    *
 *         https://github.com/maxdev1/ghost                                            *
 *                                                                                     *
 * This program is free software; you can redistribute it and/or                       *
 * modify it under the terms of the GNU General Public License                         *
 * as published by the Free Software Foundation; either version 2                      *
 * of the License, or (char *argumentat your option) any later version.                *
 *                                                                                     *
 * This program is distributed in the hope that it will be useful,                     *
 * but WITHout ANY WARRANTY; without even the implied warranty of                      *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                       *
 * GNU General Public License for more details.                                        *
 *                                                                                     *
 * You should have received a copy of the GNU General Public License                   *
 * along with this program; if not, write to the Free Software                         *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * */

#ifndef __BUTTON__
#define __BUTTON__

#include <components/ActionComponent.hpp>
#include <components/ButtonState.hpp>
#include <components/ColoredComponent.hpp>
#include <components/component.hpp>
#include <components/label.hpp>
#include <components/PngComponent.hpp>
#include <components/TitledComponent.hpp>
#include <LibGraphics/Metrics/Insets.hh>
#include <LibGraphics/Text/Font.hh>
#include <string>

/**
 *
 */
class Button_t : public Component_t,
                 public TitledComponent_t,
                 public ActionComponent_t,
                 public PngComponent_t,
                 public ColoredComponent_t {
private:
    ButtonState_t             state;
    Label_t                   label;
    Graphics::Metrics::Insets insets;
    bool                      enabled;

    cairo_t*                     cr;
    Graphics::Metrics::Rectangle bounds;

    Graphics::Color::ArgbGradient border;
    Graphics::Color::ArgbGradient previous;

public:
    /**
     *
     */
    Button_t();
    virtual ~Button_t() {}

    /*
     * Component
     */
    virtual void layout();
    virtual void paint();
    virtual bool handle(Event_t& e);
    Label_t&     getLabel() { return label; }
    virtual void setFocus(bool focus);
    virtual void handleBoundChange(Graphics::Metrics::Rectangle oldBounds);
    virtual bool getNumericProperty(int property, uint32_t* out);
    virtual bool setNumericProperty(int property, uint32_t value);

    /*
     *	titled component
     */
    virtual void        setTitle(std::string title);
    virtual std::string getTitle();
    virtual void        setTitleFont(std::string fontName);
    virtual void        setFontSize(int size);
    virtual void        setTitleAlignment(Graphics::Text::Alignment alignment);

    /*
     * png component
     */
    virtual void setPNG(std::string path, Graphics::Metrics::Point position);
    virtual void PngAnimation(std::string              path,
                              Graphics::Metrics::Point PNGstartAnimation,
                              Graphics::Metrics::Point PNGendAnimation,
                              size_t                   sleep);

    /*
     * colored component
     */
    virtual void setColor(Graphics::Color::ArgbGradient color, Graphics::Color::ArgbGradient tltColor);
};

#endif
