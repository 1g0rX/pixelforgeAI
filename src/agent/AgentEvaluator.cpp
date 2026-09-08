#include "agent/AgentEvaluator.hpp"
#include "sprite/SpriteAnalyzer.hpp"
#include "animation/FrameDiffer.hpp"
#include <sstream>

namespace pixelforge::agent {

std::string EvaluationResult::to_text_critique() const {
    if (passed) {
        return "Autoavaliação: Sprite validado com sucesso. Nenhum defeito estrutural detectado.";
    }
    std::ostringstream ss;
    ss << "Autoavaliação encontrou " << issue_count << " problema(s):\n";
    for (const auto& iss : issues) {
        ss << " - " << iss << "\n";
    }
    ss << "\nRecomendações corretivas:\n";
    for (const auto& rec : recommendations) {
        ss << " - " << rec << "\n";
    }
    return ss.str();
}

EvaluationResult AgentEvaluator::evaluate_sprite(const sprite::Sprite& sprite, size_t max_expected_colors) {
    EvaluationResult res;
    sprite::AnalysisOptions opt{.max_allowed_colors = max_expected_colors};
    auto report = sprite::SpriteAnalyzer::analyze(sprite, opt);

    if (report.out_of_bounds) {
        res.passed = false;
        res.issue_count++;
        res.issues.push_back("Pixels ou regiões ultrapassam o limite do canvas do sprite.");
        res.recommendations.push_back("Execute move_region para reposicionar elementos dentro do canvas.");
    }

    if (report.isolated_pixel_count > 0) {
        res.passed = false;
        res.issue_count++;
        res.issues.push_back("Detectados " + std::to_string(report.isolated_pixel_count) + " pixel(s) isolados (ruído).");
        res.recommendations.push_back("Chame clean_pixel_noise(threshold=1) para remover ruído órfão.");
    }

    if (report.anti_aliasing_detected) {
        res.passed = false;
        res.issue_count++;
        res.issues.push_back("Detectada transparência intermediária (anti-aliasing) imprópria para pixel art.");
        res.recommendations.push_back("Certifique-se de usar apenas cores 100% sólidas ou totalmente transparentes.");
    }

    if (report.color_count > max_expected_colors) {
        res.passed = false;
        res.issue_count++;
        res.issues.push_back("Excesso de cores: " + std::to_string(report.color_count) +
                             " cores (limite: " + std::to_string(max_expected_colors) + ").");
        res.recommendations.push_back("Chame quantize_palette(" + std::to_string(max_expected_colors) + ") para consolidar paleta.");
    }

    if (report.bounding_box.is_empty()) {
        res.passed = false;
        res.issue_count++;
        res.issues.push_back("O sprite está totalmente vazio (sem pixels visíveis).");
        res.recommendations.push_back("Desenhe formas primitivas ou preencha regiões.");
    }

    return res;
}

EvaluationResult AgentEvaluator::evaluate_animation(const animation::Animation& anim) {
    EvaluationResult res;
    if (anim.empty()) {
        res.passed = false;
        res.issue_count++;
        res.issues.push_back("A animação não contém frames.");
        res.recommendations.push_back("Chame create_animation e add_frame.");
        return res;
    }

    for (size_t i = 0; i < anim.frame_count(); ++i) {
        auto sub = evaluate_sprite(anim.frame(i).sprite);
        if (!sub.passed) {
            res.passed = false;
            res.issue_count += sub.issue_count;
            for (const auto& iss : sub.issues) {
                res.issues.push_back("Frame " + std::to_string(i) + ": " + iss);
            }
        }
    }

    // Consistency check between adjacent frames
    for (size_t i = 0; i + 1 < anim.frame_count(); ++i) {
        auto diff = animation::FrameDiffer::compare(anim.frame(i), anim.frame(i + 1), i, i + 1);
        if (diff.pixel_difference_count == 0) {
            res.issues.push_back("Frame " + std::to_string(i) + " e Frame " + std::to_string(i + 1) + " são idênticos.");
            res.recommendations.push_back("Aplique movimento ou variação na pose entre os frames.");
        }
    }

    return res;
}

} // namespace pixelforge::agent
