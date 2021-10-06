#pragma once

#include "glsl.hpp"

#include <array>
#include <memory>
#include <ostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <string>

struct DumpInfo {
    std::ostream* os;


};

struct Node {
    virtual std::string sdf_only(const std::string&) const {
        throw std::logic_error("Not implemented.");
    }

    virtual std::string sdf_and_material(const std::string&) const {
        throw std::logic_error("Not implemented.");
    }

    virtual void dump_sdf_only(std::ostream&) const {}

    virtual void dump_sdf_and_material(std::ostream&) const {}
};

using Children = std::vector<std::shared_ptr<Node>>;
using SharedNode = std::shared_ptr<Node>;

struct Branch : Node {
    std::vector<SharedNode> children;

    Branch() = default;

    Branch(SharedNode child)
        : children()
    {
        children.push_back(std::move(child));
    }

    Branch(std::vector<SharedNode>&& children)
        : children(std::move(children))
    {}

    std::string sdf_only(const std::string& s) const override {
        if(size(children) == 1) {
            return children.at(0)->sdf_only(s);
        } else {
            throw std::logic_error("Not implemented.");
        }
    }

    std::string sdf_and_material(const std::string& s) const override {
        if(size(children) == 1) {
            return children.at(0)->sdf_and_material(s);
        } else {
            throw std::logic_error("Not implemented.");
        }
    }

    void dump_sdf_only(std::ostream& os) const override {
        for(auto&& c : children) {
            c->dump_sdf_only(os);
        }
    };

    void dump_sdf_and_material(std::ostream& os) const override {
        for(auto&& c : children) {
            c->dump_sdf_and_material(os);
        }
    }
};

struct Attraction : Branch {
    float intensity = 1.f;

    Attraction(float intensity, SharedNode child)
        : Branch({std::move(child)})
        , intensity(intensity)
    {}

    std::string sdf_only(const std::string& s) const override {
        return children.at(0)->sdf_only(
            "attracted(\n" + s + ",\n" + glsl(intensity) + ")");
    }

    std::string sdf_and_material(const std::string& s) const override {
        return children.at(0)->sdf_and_material(
            "attracted(\n" + s + ",\n" + glsl(intensity) + ")");
    }
};

// struct Controlling : Branch {
//     Controlling(SharedNode child)
//         : Branch(std::move(child))
//     {}

//     std::string sdf_only(const std::string& s) const override {
//         return children.at(0)->sdf_only(
//             "(controls_transform * vec4(" + s + ", 1.)).xyz");;
//     }

//     std::string sdf_and_material(const std::string& s) const override {
//         return "(controls_transform * vec4(" + s + ", 1.)).xyz";
//     }
// };

// inline
// SharedNode controlled(SharedNode child) {
//     return std::make_shared<Controlling>(std::move(child));
// }

struct Dilatation : Branch {
    float radius = 0.f;

    Dilatation(float radius, SharedNode child)
        : Branch({std::move(child)})
        , radius(radius)
    {}

    std::string sdf_only(const std::string& s) const override {
        return "(" + children.at(0)->sdf_only(s) + " - " + glsl(radius) + ")";
    }

    std::string sdf_and_material(const std::string& s) const override {
        return "(" + children.at(0)->sdf_and_material(s) + " - " + glsl(radius) + ")";
    }
};

struct Intersection : Branch {
    Intersection(SharedNode sn0, SharedNode sn1)
        : Branch({std::move(sn0), std::move(sn1)})
    {}

    Intersection(Children cn)
        : Branch(std::move(cn))
    {}

    std::string sdf_only(const std::string& s) const override {
        auto r = children.at(0)->sdf_only(s);
        for(std::size_t i = 1; i < size(children); ++i) {
            r = "sdf_union(\n"
            + children.at(i)->sdf_only(s) + ",\n"
            + r + ")";
        }
        return r;
    }

    std::string sdf_and_material(const std::string& s) const override {
        auto r = children.at(0)->sdf_and_material(s);
        for(std::size_t i = 1; i < size(children); ++i) {
            r = "sdf_union(\n"
            + children.at(i)->sdf_and_material(s) + ",\n"
            + r + ")";
        }
        return r;
    }
};

struct Material : Branch {
    std::array<float, 3> color;

    Material(std::array<float, 3> color, SharedNode child)
        : Branch({std::move(child)})
        , color(color)
    {}

    std::string sdf_and_material(const std::string& s) const override {
        return "SdfAndMaterial(\n"
        + sdf_only(s) + ",\n"
        + glsl(color)
        + ")";
    }
};

struct Object : Branch {
    std::string name;

    Object(std::string name, SharedNode child)
        : Branch({std::move(child)})
        , name(std::move(name))
    {}

    std::string sdf_only(const std::string& s) const override {
        return name + "_sdf(" + s + ")";
    }

    std::string sdf_and_material(const std::string& s) const override {
        return name + "_material(" + s + ")";
    }

    void dump_sdf_only(std::ostream& os) const override {
        Branch::dump_sdf_only(os);
        os << "float " << name << "_sdf(vec3 position) {\n"
        << "    return " << children.at(0)->sdf_only("position") << ";\n"
        << "}\n\n";
    }

    void dump_sdf_and_material(std::ostream& os) const override {
        Branch::dump_sdf_and_material(os);
        os << "SdfAndMaterial " << name << "_material(vec3 position) {\n"
        << "    return " << children.at(0)->sdf_and_material("position") << ";\n"
        << "}\n\n";
    }
};

struct Onion : Branch {
    Onion(SharedNode child)
        : Branch({std::move(child)})
    {}

    std::string sdf_only(const std::string& s) const override {
        return "onion(" + children.at(0)->sdf_only(s) + ")";
    }

    std::string sdf_and_material(const std::string& s) const override {
        return "onion(" + children.at(0)->sdf_and_material(s) + ")";
    }
};

SharedNode onion(SharedNode sn) {
    return std::make_shared<Onion>(std::move(sn));
}

struct RotatedX : Branch {
    float angle = 0.f;

    RotatedX(float angle, SharedNode child)
        : Branch({std::move(child)})
        , angle(angle)
    {}

    std::string sdf_only(const std::string& s) const override {
        return children.at(0)->sdf_only(
            "rotated_x(" + s + ", " + glsl(angle) + ")");
    }

    std::string sdf_and_material(const std::string& s) const override {
        return children.at(0)->sdf_and_material(
            "rotated_x(" + s + ", " + glsl(angle) + ")");
    }
};

struct RotatedY : Branch {
    float angle = 0.f;

    RotatedY(float angle, SharedNode child)
        : Branch({std::move(child)})
        , angle(angle)
    {}

    std::string sdf_only(const std::string& s) const override {
        return children.at(0)->sdf_only(
            "rotated_y(" + s + ", " + glsl(angle) + ")");
    }

    std::string sdf_and_material(const std::string& s) const override {
        return children.at(0)->sdf_and_material(
            "rotated_y(" + s + ", " + glsl(angle) + ")");
    }
};

struct RotatedZ : Branch {
    float angle = 0.f;

    RotatedZ(float angle, SharedNode child)
        : Branch({std::move(child)})
        , angle(angle)
    {}

    std::string sdf_only(const std::string& s) const override {
        return children.at(0)->sdf_only(
            "rotated_z(" + s + ", " + glsl(angle) + ")");
    }

    std::string sdf_and_material(const std::string& s) const override {
        return children.at(0)->sdf_and_material(
            "rotated_z(" + s + ", " + glsl(angle) + ")");
    }
};

struct Scaling : Branch {
    std::array<float, 3> scaling = {1.f, 1.f, 1.f};

    Scaling(std::array<float, 3> scaling, SharedNode child)
        : Branch({std::move(child)})
        , scaling(scaling)
    {}

    std::string sdf_only(const std::string& s) const override {
        auto ms = 0.f;
        for(auto sc : scaling) {
            ms = std::max(ms, std::abs(sc));
        }
        return "scaling_out("
        + children.at(0)->sdf_only(
            "scaling_in(" + s + ", " + glsl(scaling) + ")")
        + ", " + glsl(ms) + ")";
    }

    std::string sdf_and_material(const std::string& s) const override {
        auto ms = 0.f;
        for(auto sc : scaling) {
            ms = std::max(ms, std::abs(sc));
        }
        return "scaling_out("
        + children.at(0)->sdf_and_material(
            "scaling_in(" + s + ", " + glsl(scaling) + ")")
        + ", " + glsl(ms) + ")";
    }
};

struct Swizzling : Branch {
    std::array<int, 3> swizzling = {0, 1, 2};

    Swizzling(std::array<int, 3> swizzling, SharedNode child)
        : Branch({std::move(child)})
        , swizzling(swizzling)
    {}

    std::string sdf_only(const std::string& s) const override {
        auto symbols = std::array{'x', 'y', 'z'};
        auto ss = s + ".";
        for(auto sw : swizzling) {
            ss += symbols[sw];
        }
        return children.at(0)->sdf_only(ss);
    }

    std::string sdf_and_material(const std::string& s) const override {
        auto symbols = std::array{'x', 'y', 'z'};
        auto ss = s + ".";
        for(auto sw : swizzling) {
            ss += symbols[sw];
        }
        return children.at(0)->sdf_and_material(ss);
    }
};

struct Translation : Branch {
    std::array<float, 3> translation;

    Translation(std::array<float, 3> translation, SharedNode child)
        : Branch({std::move(child)})
        , translation(translation)
    {}

    std::string sdf_only(const std::string& s) const override {
        return children.at(0)->sdf_only(
            "translated(" + s + ", " + glsl(translation) + ")");
    }

    std::string sdf_and_material(const std::string& s) const override {
        return children.at(0)->sdf_and_material(
            "translated(" + s + ", " + glsl(translation) + ")");
    }
};

struct UniformScaling : Branch {
    float scaling = 1.f;

    UniformScaling(float scaling, SharedNode child)
        : Branch({std::move(child)})
        , scaling(scaling)
    {}

    std::string sdf_only(const std::string& s) const override {
        return "scaling_out("
        + children.at(0)->sdf_only(
            "scaling_in(" + s + ", " + glsl(scaling) + ")")
        + ", " + glsl(scaling) + ")";
    }

    std::string sdf_and_material(const std::string& s) const override {
        return "scaling_out("
        + children.at(0)->sdf_and_material(
            "scaling_in(" + s + ", " + glsl(scaling) + ")")
        + ", " + glsl(scaling) + ")";
    }
};

struct Union : Branch {
    Union(SharedNode sn0, SharedNode sn1)
        : Branch({std::move(sn0), std::move(sn1)})
    {}

    Union(Children cn)
        : Branch(std::move(cn))
    {}

    std::string sdf_only(const std::string& s) const override {
        auto r = children.at(0)->sdf_only(s);
        for(std::size_t i = 1; i < size(children); ++i) {
            r = "sdf_union(\n"
            + children.at(i)->sdf_only(s) + ",\n"
            + r + ")";
        }
        return r;
    }

    std::string sdf_and_material(const std::string& s) const override {
        auto r = children.at(0)->sdf_and_material(s);
        for(std::size_t i = 1; i < size(children); ++i) {
            r = "sdf_union(\n"
            + children.at(i)->sdf_and_material(s) + ",\n"
            + r + ")";
        }
        return r;
    }
};

template<typename... SharedNodes>
SharedNode unionn(SharedNodes... sns) {
    return std::make_shared<Union>(
        std::vector<SharedNode>{std::move(sns)...});
}

struct Leaf : Node {};

struct Cube : Leaf {
    std::string sdf_only(const std::string& s) const override {
        return "sdf_cube(" + s + ")";
    }
};

inline
SharedNode cube() {
    return std::make_shared<Cube>();
}

struct Ellipsoid : Leaf {
    std::array<float, 3> radiuses = {1.f, 1.f, 1.f};

    Ellipsoid(std::array<float, 3> radiuses)
        : radiuses(radiuses)
    {}

    std::string sdf_only(const std::string& s) const override {
        return "sdf_ellipsoid("
        + s + ", "
        + glsl(radiuses) + ")";
    }
};

struct LineSegment : Leaf {
    std::array<float, 3> position0;
    std::array<float, 3> position1;

    LineSegment(
        std::array<float, 3> position0,
        std::array<float, 3> position1)
        : position0(position0)
        , position1(position1)
    {}

    std::string sdf_only(const std::string& s) const override {
        return "sdf_line_segment(\n"
        + glsl(position0) + ",\n"
        + glsl(position1) + ",\n"
        + s + ")";
    }
};

struct Plane : Leaf {
    float width = 1.f;

    Plane(float width)
        : width(width)
    {}

    std::string sdf_only(const std::string& s) const override {
        return "(sdf_plane(" + s + ") - " + glsl(width / 2.f) + ")";
    }
};

struct Sphere : Leaf {
    float radius = 1.f;

    Sphere(float r)
        : radius(r)
    {}

    std::string sdf_only(const std::string& s) const override {
        auto ss = std::stringstream();
        ss << "(sdf_point("
        << s << ") - "
        << radius << ")";
        return ss.str();
    }
};
