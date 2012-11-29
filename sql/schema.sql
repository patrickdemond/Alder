SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='';

DROP SCHEMA IF EXISTS `alder` ;
CREATE SCHEMA IF NOT EXISTS `alder` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci ;
USE `alder` ;

-- -----------------------------------------------------
-- Table `alder`.`Study`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`Study` ;

CREATE  TABLE IF NOT EXISTS `alder`.`Study` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `create_timestamp` TIMESTAMP NOT NULL ,
  `uid` VARCHAR(45) NOT NULL ,
  `site` VARCHAR(45) NOT NULL ,
  `interviewer` VARCHAR(45) NOT NULL ,
  `datetime_acquired` DATETIME NOT NULL ,
  `datetime_reviewed` DATETIME NULL ,
  `note` TEXT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `dk_uid` (`uid` ASC) ,
  INDEX `dk_datetime_reviewed` (`datetime_reviewed` ASC) ,
  INDEX `dk_site` (`site` ASC) ,
  INDEX `dk_datetime_acquired` (`datetime_acquired` ASC) ,
  INDEX `dk_interviewer` (`interviewer` ASC) ,
  UNIQUE INDEX `uq_uid` (`uid` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`Exam`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`Exam` ;

CREATE  TABLE IF NOT EXISTS `alder`.`Exam` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `create_timestamp` TIMESTAMP NOT NULL ,
  `study_id` INT UNSIGNED NOT NULL ,
  `laterality` ENUM('right','left') NOT NULL ,
  `type` ENUM('cimt','plaque') NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_exam_study_id` (`study_id` ASC) ,
  INDEX `dk_laterality` (`laterality` ASC) ,
  INDEX `dk_type` (`type` ASC) ,
  CONSTRAINT `fk_exam_study_id`
    FOREIGN KEY (`study_id` )
    REFERENCES `alder`.`Study` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`Cineloop`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`Cineloop` ;

CREATE  TABLE IF NOT EXISTS `alder`.`Cineloop` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `create_timestamp` TIMESTAMP NOT NULL ,
  `exam_id` INT UNSIGNED NOT NULL ,
  `number` INT NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_cineloop_exam_id` (`exam_id` ASC) ,
  CONSTRAINT `fk_cineloop_exam_id`
    FOREIGN KEY (`exam_id` )
    REFERENCES `alder`.`Exam` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`Image`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`Image` ;

CREATE  TABLE IF NOT EXISTS `alder`.`Image` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `create_timestamp` TIMESTAMP NOT NULL ,
  `exam_id` INT UNSIGNED NOT NULL ,
  `cineloop_id` INT UNSIGNED NOT NULL ,
  `frame` INT NOT NULL ,
  `interviewer_defined` TINYINT(1) NOT NULL ,
  `min` FLOAT NULL ,
  `max` FLOAT NULL ,
  `mean` FLOAT NULL ,
  `sd` FLOAT NULL ,
  `n` INT(11) NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_image_exam_id` (`exam_id` ASC) ,
  INDEX `fk_image_cineloop_id` (`cineloop_id` ASC) ,
  INDEX `dk_cineloop_id_frame` (`cineloop_id` ASC, `frame` ASC) ,
  CONSTRAINT `fk_image_exam_id`
    FOREIGN KEY (`exam_id` )
    REFERENCES `alder`.`Exam` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_image_cineloop_id`
    FOREIGN KEY (`cineloop_id` )
    REFERENCES `alder`.`Cineloop` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`User`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`User` ;

CREATE  TABLE IF NOT EXISTS `alder`.`User` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `create_timestamp` TIMESTAMP NOT NULL ,
  `name` VARCHAR(255) NOT NULL ,
  `password` VARCHAR(255) NOT NULL ,
  `last_login` DATETIME NULL ,
  `study_id` INT UNSIGNED NULL ,
  PRIMARY KEY (`id`) ,
  UNIQUE INDEX `uq_name` (`name` ASC) ,
  INDEX `dk_last_login` (`last_login` ASC) ,
  INDEX `fk_study_id` (`study_id` ASC) ,
  CONSTRAINT `fk_user_study_id`
    FOREIGN KEY (`study_id` )
    REFERENCES `alder`.`Study` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`Rating`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`Rating` ;

CREATE  TABLE IF NOT EXISTS `alder`.`Rating` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `create_timestamp` TIMESTAMP NOT NULL ,
  `image_id` INT UNSIGNED NOT NULL ,
  `user_id` INT UNSIGNED NOT NULL ,
  `rating` TINYINT(1) NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_rating_image_id` (`image_id` ASC) ,
  INDEX `fk_rating_user_id` (`user_id` ASC) ,
  INDEX `dk_rating` (`rating` ASC) ,
  CONSTRAINT `fk_rating_image_id`
    FOREIGN KEY (`image_id` )
    REFERENCES `alder`.`Image` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_rating_user_id`
    FOREIGN KEY (`user_id` )
    REFERENCES `alder`.`User` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;



SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
